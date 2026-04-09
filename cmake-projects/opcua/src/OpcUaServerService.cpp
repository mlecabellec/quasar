#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/opcua/UA_Common.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/named/IObserver.hpp"
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <stdexcept>

namespace quasar::opcua {

using namespace named;

class OpcUaServerService::TreeObserver : public named::IObserver {
public:
    explicit TreeObserver(OpcUaServerService& service) : m_service(service) {}
    
    void notify(std::shared_ptr<NamedObject> eventData) override {
        m_service.handleObjectChanged(eventData);
    }
private:
    OpcUaServerService& m_service;
};

OpcUaServerService::OpcUaServerService(const std::string& name)
    : NamedService(name) {
    m_observer = std::make_shared<TreeObserver>(*this);
}

OpcUaServerService::~OpcUaServerService() {
    stop();
}

std::shared_ptr<OpcUaServerService> OpcUaServerService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    struct make_shared_enabler : public OpcUaServerService {
        explicit make_shared_enabler(const std::string& n) : OpcUaServerService(n) {}
    };
    std::shared_ptr<OpcUaServerService> self = std::make_shared<make_shared_enabler>(name);
    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }
    
    std::weak_ptr<OpcUaServerService> weakSelf = self;
    NamedMethod::create("run", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<OpcUaServerService> s = weakSelf.lock()) {
            if (s->m_server) {
                UA_Server_run_iterate(s->m_server, false);
            }
        }
        return nullptr;
    }, self);

    return self;
}

void OpcUaServerService::setPort(uint16_t port) {
    m_port = port;
}

void OpcUaServerService::setRootObject(std::shared_ptr<NamedObject> root) {
    m_rootObject = root;
}

void OpcUaServerService::start() {
    if (isRunning()) return;

    initializeOpcUa();
    setCycleTime(std::chrono::milliseconds(1));
    NamedService::start();
}

void OpcUaServerService::stop() {
    if (!isRunning()) return;

    NamedService::stop();

    if (m_server) {
        UA_Server_run_shutdown(m_server);
        UA_Server_delete(m_server);
        m_server = nullptr;
    }
}


std::string OpcUaServerService::getType() const {
    return "OpcUaServerService";
}

void OpcUaServerService::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    m_securityManager.loadCertificate(certPath, keyPath);
}

void OpcUaServerService::loadTrustList(const std::vector<std::string>& trustListPaths) {
    m_securityManager.loadTrustList(trustListPaths);
}

void OpcUaServerService::addUser(const std::string& username, const std::string& password) {
    m_users[username] = password;
}

void OpcUaServerService::setAllowAnonymous(bool allow) {
    m_allowAnonymous = allow;
}

void OpcUaServerService::initializeOpcUa() {
    m_server = UA_Server_new();
    UA_ServerConfig* config = UA_Server_getConfig(m_server);
    UA_ServerConfig_setMinimal(config, m_port, nullptr);
    
    // Configure security via the manager
    m_securityManager.configureServer(m_server, config);

    // Configure Access Control
    if (!m_users.empty() || !m_allowAnonymous) {
        size_t userCount = m_users.size();
        UA_UsernamePasswordLogin* logins = (UA_UsernamePasswordLogin*)UA_malloc(sizeof(UA_UsernamePasswordLogin) * userCount);
        size_t i = 0;
        for (auto const& [u, p] : m_users) {
            logins[i].username = UA_String_fromStdString(u);
            logins[i].password = UA_ByteString_fromStdString(p);
            i++;
        }
        UA_AccessControl_default(config, m_allowAnonymous, nullptr, userCount, logins);
    }

    std::shared_ptr<NamedObject> root = m_rootObject ? m_rootObject : getSelf();
    mapObject(root, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    UA_Server_run_startup(m_server);
}


void OpcUaServerService::onWrite(UA_Server *server, const UA_NodeId *sessionId,
                                 void *sessionContext, const UA_NodeId *nodeId,
                                 void *nodeContext, const UA_NumericRange *range,
                                 const UA_DataValue *data) {
    (void)server; (void)sessionId; (void)sessionContext; (void)nodeId; (void)range;
    if (nodeContext && data->hasValue) {
        NamedObject* obj = static_cast<NamedObject*>(nodeContext);
        fromUaVariant(&data->value, obj->getSelf());
    }
}

UA_StatusCode OpcUaServerService::onMethodCall(UA_Server *server, const UA_NodeId *sessionId,
                                                void *sessionContext, const UA_NodeId *methodId,
                                                void *methodContext, const UA_NodeId *objectId,
                                                void *objectContext, size_t inputSize,
                                                const UA_Variant *input, size_t outputSize,
                                                UA_Variant *output) {
    (void)server; (void)sessionId; (void)sessionContext; (void)methodId; (void)methodContext; (void)objectId; (void)objectContext;
    if (!methodContext || inputSize == 0 || outputSize == 0) return UA_STATUSCODE_BADINTERNALERROR;

    NamedMethod* method = static_cast<NamedMethod*>(methodContext);
    
    if (input[0].type != &UA_TYPES[UA_TYPES_STRING]) return UA_STATUSCODE_BADTYPEMISMATCH;
    UA_String uas = *(UA_String*)input[0].data;
    std::string jsonArgs((char*)uas.data, uas.length);

    std::shared_ptr<NamedObject> args = nullptr;
    if (jsonArgs != "null") {
        args = serialization::fromJson(jsonArgs);
    }

    std::shared_ptr<NamedObject> resObj = method->execute(args);
    std::string jsonRes = serialization::toJson(resObj);
    
    UA_String uares = UA_STRING_ALLOC(jsonRes.c_str());
    UA_Variant_setScalarCopy(output, &uares, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&uares);

    return UA_STATUSCODE_GOOD;
}

void OpcUaServerService::mapObject(std::shared_ptr<NamedObject> obj, UA_NodeId parentNodeId) {
    if (!m_server || !obj) return;

    std::string objName = obj->getName();
    std::string type = obj->getType();
    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(1, objName.c_str());
    UA_LocalizedText displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", objName.c_str());

    UA_NodeId newNodeId;

    if (type == "NamedMethod" || type == "NamedLuaMethod") {
        UA_MethodAttributes mAttr = UA_MethodAttributes_default;
        mAttr.displayName = displayName;
        mAttr.executable = true;
        mAttr.userExecutable = true;
        
        UA_Argument inputArgument;
        UA_Argument_init(&inputArgument);
        inputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "JSON Arguments");
        inputArgument.name = UA_STRING_ALLOC("Args");
        inputArgument.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
        inputArgument.valueRank = UA_VALUERANK_SCALAR;

        UA_Argument outputArgument;
        UA_Argument_init(&outputArgument);
        outputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "JSON Result");
        outputArgument.name = UA_STRING_ALLOC("Result");
        outputArgument.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
        outputArgument.valueRank = UA_VALUERANK_SCALAR;


        UA_Server_addMethodNode(m_server, UA_NODEID_NULL, parentNodeId,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                browseName, mAttr, &onMethodCall,
                                1, &inputArgument, 1, &outputArgument,
                                obj.get(), &newNodeId);
        
        // Ensure the methodContext is also set to obj.get() if it was not handled by the previous call
        // Actually, open62541 uses the last argument as both nodeContext and methodContext in some versions,
        // but explicitly setting it via UA_Server_setNodeContext is safer if needed.
        // However, looking at UA_Server_addMethodNode signature:
        // UA_Server_addMethodNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
        //                        const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
        //                        const UA_QualifiedName browseName, const UA_MethodAttributes attr,
        //                        UA_MethodCallback method,
        //                        size_t inputArgumentsSize, const UA_Argument *inputArguments,
        //                        size_t outputArgumentsSize, const UA_Argument *outputArguments,
        //                        void *nodeContext, UA_NodeId *outNewNodeId)
        // In open62541, nodeContext IS what's passed as methodContext to the callback.
        
        UA_String_clear(&inputArgument.name);
        UA_LocalizedText_clear(&inputArgument.description);
        UA_String_clear(&outputArgument.name);
        UA_LocalizedText_clear(&outputArgument.description);
    } else if (type == "NamedInteger" || type == "NamedBoolean" || type == "NamedString" || type == "NamedFloatingPoint") {
        UA_VariableAttributes vAttr = UA_VariableAttributes_default;
        vAttr.displayName = displayName;
        vAttr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", type.c_str());
        vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        
        UA_Variant value = toUaVariant(obj);
        vAttr.value = value;
        
        UA_Server_addVariableNode(m_server, UA_NODEID_NULL, parentNodeId,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                  browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                  vAttr, obj.get(), &newNodeId);
        
        UA_ValueCallback callback;
        callback.onRead = nullptr;
        callback.onWrite = onWrite;
        UA_Server_setVariableNode_valueCallback(m_server, newNodeId, callback);
        
        UA_Variant_clear(&value);
    } else {
        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName = displayName;
        UA_Server_addObjectNode(m_server, UA_NODEID_NULL, parentNodeId,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, obj.get(), &newNodeId);
    }

    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&displayName);

    if (!UA_NodeId_isNull(&newNodeId)) {
        m_objectToNodeMap[obj] = newNodeId;
        obj->subscribe(m_observer);
    }


    for (auto const& child : obj->getChildren()) {
        mapObject(child, newNodeId);
    }
}

void OpcUaServerService::handleObjectChanged(std::shared_ptr<named::NamedObject> obj) {
    if (!m_server || !obj) return;
    
    auto it = m_objectToNodeMap.find(obj);
    if (it != m_objectToNodeMap.end()) {
        UA_Variant value = toUaVariant(obj);
        if (!UA_Variant_isEmpty(&value)) {
            UA_Server_writeValue(m_server, it->second, value);
        }
        UA_Variant_clear(&value);
    }
}

} // namespace quasar::opcua
