#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/UA_Common.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
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

static std::shared_ptr<NamedObject> createLocalFromRemote(const UA_QualifiedName& browseName, const UA_Variant* value) {
    std::string name((char*)browseName.name.data, browseName.name.length);
    if (!value || UA_Variant_isEmpty(value)) {
        return NamedObject::create(name);
    }
    
    if (value->type == &UA_TYPES[UA_TYPES_INT32]) {
        return NamedInteger<int32_t>::create(name, *(UA_Int32*)value->data);
    } else if (value->type == &UA_TYPES[UA_TYPES_INT64]) {
        return NamedInteger<int64_t>::create(name, *(UA_Int64*)value->data);
    } else if (value->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
        return NamedBoolean::create(name, *(UA_Boolean*)value->data);
    } else if (value->type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String uas = *(UA_String*)value->data;
        return NamedString::create(name, std::string((char*)uas.data, uas.length));
    } else if (value->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        return NamedFloatingPoint<double>::create(name, *(UA_Double*)value->data);
    }
    
    return NamedObject::create(name);
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
            
            UA_Variant value;
            UA_Variant_init(&value);
            UA_Client_readValueAttribute(m_client, ref->nodeId.nodeId, &value);
            
            std::shared_ptr<NamedObject> localObj = createLocalFromRemote(ref->browseName, &value);
            localObj->setParent(localParent);
            
            if (ref->nodeClass == UA_NODECLASS_VARIABLE) {
                // Setup subscription and monitored item
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
            
            UA_Variant_clear(&value);
        }
    }
    
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bRes);
}

} // namespace quasar::opcua
