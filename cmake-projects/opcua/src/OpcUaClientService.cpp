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
                s->processTasks();
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

void OpcUaClientService::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    m_securityManager.loadCertificate(certPath, keyPath);
}

void OpcUaClientService::loadTrustList(const std::vector<std::string>& trustListPaths) {
    m_securityManager.loadTrustList(trustListPaths);
}

void OpcUaClientService::setCredentials(const std::string& username, const std::string& password) {
    m_username = username;
    m_password = password;
}

void OpcUaClientService::processTasks() {
    std::queue<std::function<void(UA_Client*)>> currentTasks;
    {
        std::unique_lock<std::timed_mutex> lock(m_queueMutex, std::chrono::milliseconds(5000));
        if (lock.owns_lock()) {
            std::swap(currentTasks, m_tasks);
        }
    }
    
    while (!currentTasks.empty()) {
        currentTasks.front()(m_client);
        currentTasks.pop();
    }
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
    UA_ClientConfig* config = UA_Client_getConfig(m_client);
    UA_ClientConfig_setDefault(config);
    config->allowNonePolicyPassword = true;

    // Configure security via the manager
    m_securityManager.configureClient(m_client);

    // Configure Credentials
    if (!m_username.empty()) {
        UA_ClientConfig_setAuthenticationUsername(config, m_username.c_str(), m_password.c_str());
    }

    UA_StatusCode retval = UA_Client_connect(m_client, m_url.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(m_client);
        m_client = nullptr;
        throw std::runtime_error("Failed to connect to OPC UA server: " + m_url);
    }

    // Create a single subscription for all variables
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(m_client, request,
                                                                            nullptr, nullptr, nullptr);
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        m_subscriptionId = response.subscriptionId;
    }

    browseAndMirror(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), getSelf());
}

void OpcUaClientService::browseAndMirror(UA_NodeId remoteNodeId, std::shared_ptr<NamedObject> localParent) {
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    UA_NodeId_copy(&remoteNodeId, &bReq.nodesToBrowse[0].nodeId);
    bReq.nodesToBrowse[0].referenceTypeId = UA_NODEID_NULL;
    bReq.nodesToBrowse[0].includeSubtypes = true;
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResponse bRes = UA_Client_Service_browse(m_client, bReq);
    
    for (size_t i = 0; i < bRes.resultsSize; ++i) {
        for (size_t j = 0; j < bRes.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &bRes.results[i].references[j];
            
            UA_NodeId targetId;
            UA_NodeId_init(&targetId);
            UA_NodeId_copy(&ref->nodeId.nodeId, &targetId);

            std::string rawName((char*)ref->browseName.name.data, ref->browseName.name.length);
            printf("[C++] Found ref: %s (ns=%d, class=%d)\n", rawName.c_str(), targetId.namespaceIndex, ref->nodeClass);
            std::string name = sanitizeName(rawName);
            
            if (targetId.namespaceIndex == 0 && 
                (rawName == "Server" || rawName == "Types" || rawName == "Views")) {
                UA_NodeId_clear(&targetId);
                continue;
            }

            std::shared_ptr<NamedObject> localObj;
            
            if (ref->nodeClass == UA_NODECLASS_METHOD) {
                UA_NodeId methodId;
                UA_NodeId_copy(&targetId, &methodId);
                UA_NodeId objectId;
                UA_NodeId_copy(&remoteNodeId, &objectId);
                
                std::weak_ptr<OpcUaClientService> weakSvc = std::dynamic_pointer_cast<OpcUaClientService>(getSelf());
                localObj = NamedMethod::create(name, [weakSvc, methodId, objectId](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
                    (void)owner;
                    std::shared_ptr<OpcUaClientService> s = weakSvc.lock();
                    if (!s || !s->isRunning()) return nullptr;
                    
                    std::function<std::shared_ptr<NamedObject>(UA_Client*)> task = [methodId, objectId, args](UA_Client* client) -> std::shared_ptr<NamedObject> {
                        UA_Variant input;
                        UA_Variant_init(&input);
                        std::string jsonArgs = serialization::toJson(args);
                        UA_String uas = UA_STRING_ALLOC(jsonArgs.c_str());
                        UA_Variant_setScalarCopy(&input, &uas, &UA_TYPES[UA_TYPES_STRING]);
                        UA_String_clear(&uas);
                        
                        size_t outputSize = 0;
                        UA_Variant *output = nullptr;
                        UA_StatusCode retval = UA_Client_call(client, objectId, methodId, 1, &input, &outputSize, &output);
                        UA_Variant_clear(&input);
                        
                        std::shared_ptr<NamedObject> res = nullptr;
                        if (retval == UA_STATUSCODE_GOOD && outputSize > 0 && output[0].type == &UA_TYPES[UA_TYPES_STRING]) {
                            UA_String uares = *(UA_String*)output[0].data;
                            std::string jsonRes((char*)uares.data, uares.length);
                            if (jsonRes != "null") {
                                res = serialization::fromJson(jsonRes);
                            }
                        }
                        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                        
                        // [FE-0130.4.3] Ensure we return a valid NamedObject for Lua, even if method had no output
                        if (!res) {
                            res = NamedObject::create("void");
                        }
                        return res;
                    };

                    try {
                        std::future<std::shared_ptr<NamedObject>> future = s->enqueueTask<std::shared_ptr<NamedObject>>(task);
                        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
                            std::shared_ptr<NamedObject> r = future.get();
                            return r ? r : NamedObject::create("void");
                        } else {
                            return NamedObject::create("timeout");
                        }
                    } catch (...) {
                        return NamedObject::create("error");
                    }
                }, localParent);
            } else {
                UA_Variant value;
                UA_Variant_init(&value);
                UA_Client_readValueAttribute(m_client, targetId, &value);
                
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
                if (m_subscriptionId != 0) {
                    UA_MonitoredItemCreateRequest monRequest =
                        UA_MonitoredItemCreateRequest_default(targetId);
                    UA_Client_MonitoredItems_createDataChange(m_client, m_subscriptionId,
                                                              UA_TIMESTAMPSTORETURN_BOTH, monRequest,
                                                              localObj.get(), onDataChange, nullptr);
                }
            }
            
            if (ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE) {
                // Recursively mirror children if they are not system objects
                if (!(targetId.namespaceIndex == 0 && targetId.identifierType == UA_NODEIDTYPE_NUMERIC && targetId.identifier.numeric < 100)) {
                    browseAndMirror(targetId, localObj);
                }
            }
            UA_NodeId_clear(&targetId);
        }
    }
    
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bRes);
}

} // namespace quasar::opcua
