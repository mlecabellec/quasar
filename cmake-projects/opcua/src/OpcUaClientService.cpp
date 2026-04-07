#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/UA_Common.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include <open62541/client_highlevel.h>
#include <stdexcept>
#include <sstream>

namespace quasar::opcua {

using namespace named;

OpcUaClientService::OpcUaClientService(const std::string& name)
    : NamedService(name) {
}

OpcUaClientService::~OpcUaClientService() {
    stop();
}

std::shared_ptr<OpcUaClientService> OpcUaClientService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    struct make_shared_enabler : public OpcUaClientService {
        explicit make_shared_enabler(const std::string& n) : OpcUaClientService(n) {}
    };
    std::shared_ptr<OpcUaClientService> self = std::make_shared<make_shared_enabler>(name);
    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }
    
    std::weak_ptr<OpcUaClientService> weakSelf = self;
    NamedMethod::create("run", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<OpcUaClientService> s = weakSelf.lock()) {
            if (s->m_client) {
                UA_Client_run_iterate(s->m_client, 0);
            }
        }
        return nullptr;
    }, self);

    return self;
}

void OpcUaClientService::setUrl(const std::string& url) {
    m_url = url;
}

void OpcUaClientService::start() {
    if (isRunning()) return;

    initializeClient();
    setCycleTime(std::chrono::milliseconds(10));
    NamedService::start();
}

void OpcUaClientService::stop() {
    if (!isRunning()) return;

    NamedService::stop();

    if (m_client) {
        UA_Client_disconnect(m_client);
        UA_Client_delete(m_client);
        m_client = nullptr;
    }
}

std::string OpcUaClientService::getType() const {
    return "OpcUaClientService";
}

static std::string sanitizeName(const std::string& name) {
    if (name.empty()) return "unnamed";
    std::string res;
    for (char c : name) {
        if (std::isalnum(c) || c == '_') {
            res += c;
        } else {
            res += '_';
        }
    }
    if (std::isdigit(res[0])) {
        res = "_" + res;
    }
    return res;
}

void OpcUaClientService::onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                                      UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    (void)client; (void)subId; (void)subContext; (void)monId;
    if (monContext && value->hasValue) {
        NamedObject* obj = static_cast<NamedObject*>(monContext);
        fromUaVariant(&value->value, obj->getSelf());
    }
}

void OpcUaClientService::initializeClient() {
    m_client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(m_client));
    
    UA_StatusCode retval = UA_Client_connect(m_client, m_url.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(m_client);
        m_client = nullptr;
        throw std::runtime_error("Failed to connect to OPC UA server: " + m_url);
    }

    browseAndMirror(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), getSelf());
}

void OpcUaClientService::browseAndMirror(UA_NodeId remoteNodeId, std::shared_ptr<NamedObject> localParent) {
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = remoteNodeId;
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse bRes = UA_Client_Service_browse(m_client, bReq);
    
    for (size_t i = 0; i < bRes.resultsSize; ++i) {
        for (size_t j = 0; j < bRes.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &bRes.results[i].references[j];
            
            std::string rawName((char*)ref->browseName.name.data, ref->browseName.name.length);
            std::string name = sanitizeName(rawName);
            
            std::shared_ptr<NamedObject> localObj;
            
            if (ref->nodeClass == UA_NODECLASS_METHOD) {
                UA_NodeId methodId;
                UA_NodeId_copy(&ref->nodeId.nodeId, &methodId);
                UA_NodeId objectId;
                UA_NodeId_copy(&remoteNodeId, &objectId);
                
                std::weak_ptr<OpcUaClientService> weakSelf = std::dynamic_pointer_cast<OpcUaClientService>(getSelf());
                localObj = NamedMethod::create(name, [weakSelf, methodId, objectId](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
                    auto s = weakSelf.lock();
                    if (!s || !s->m_client) return nullptr;
                    
                    UA_Variant input;
                    UA_Variant_init(&input);
                    std::string jsonArgs = serialization::toJson(args);
                    UA_String uas = UA_STRING_ALLOC(jsonArgs.c_str());
                    UA_Variant_setScalarCopy(&input, &uas, &UA_TYPES[UA_TYPES_STRING]);
                    UA_String_clear(&uas);
                    
                    size_t outputSize = 0;
                    UA_Variant *output = nullptr;
                    
                    UA_StatusCode retval = UA_Client_call(s->m_client, objectId, methodId, 1, &input, &outputSize, &output);


                    UA_Variant_clear(&input);
                    
                    if (retval != UA_STATUSCODE_GOOD) {
                        printf("[C++] Method call failed with status: 0x%08X\n", retval);
                    }


                    
                    std::shared_ptr<NamedObject> res = nullptr;
                    if (retval == UA_STATUSCODE_GOOD && outputSize > 0 && output[0].type == &UA_TYPES[UA_TYPES_STRING]) {
                        UA_String uares = *(UA_String*)output[0].data;
                        std::string jsonRes((char*)uares.data, uares.length);
                        if (jsonRes != "null") {
                            res = serialization::fromJson(jsonRes);
                        }
                    }
                    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                    return res;
                }, localParent);
            } else {
                UA_Variant value;
                UA_Variant_init(&value);
                UA_Client_readValueAttribute(m_client, ref->nodeId.nodeId, &value);
                
                // Simplified creation logic here for now
                if (value.type == &UA_TYPES[UA_TYPES_INT32]) {
                    localObj = NamedInteger<int32_t>::create(name, *(UA_Int32*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_INT64]) {
                    localObj = NamedInteger<int64_t>::create(name, *(UA_Int64*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                    localObj = NamedBoolean::create(name, *(UA_Boolean*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_STRING]) {
                    UA_String uas = *(UA_String*)value.data;
                    localObj = NamedString::create(name, std::string((char*)uas.data, uas.length));
                } else if (value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
                    localObj = NamedFloatingPoint<double>::create(name, *(UA_Double*)value.data);
                } else {
                    localObj = NamedObject::create(name);
                }
                UA_Variant_clear(&value);
                localObj->setParent(localParent);
            }
            
            if (ref->nodeClass == UA_NODECLASS_VARIABLE) {
                UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
                UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(m_client, request,
                                                                                        nullptr, nullptr, nullptr);
                if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
                    UA_MonitoredItemCreateRequest monRequest =
                        UA_MonitoredItemCreateRequest_default(ref->nodeId.nodeId);
                    UA_Client_MonitoredItems_createDataChange(m_client, response.subscriptionId,
                                                              UA_TIMESTAMPSTORETURN_BOTH, monRequest,
                                                              localObj.get(), onDataChange, nullptr);
                }
            }
            
            if (ref->nodeClass == UA_NODECLASS_OBJECT) {
                browseAndMirror(ref->nodeId.nodeId, localObj);
            }
        }
    }
    
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bRes);
}

} // namespace quasar::opcua
