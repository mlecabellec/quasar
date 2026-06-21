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

/// @brief TreeObserver implementation for handling Quasar tree updates.
class OpcUaServerService::TreeObserver : public named::IObserver {
public:
    /// @brief Construct a TreeObserver.
    /// @param service The server service instance.
    explicit TreeObserver(OpcUaServerService& service) : m_service(service) {}
    
    /// @brief Callback notified when a NamedObject changes.
    /// @param eventData The changed object.
    void notify(std::shared_ptr<NamedObject> eventData) override {
        m_service.handleObjectChanged(eventData);
    }
private:
    OpcUaServerService& m_service;
};

/// @brief Constructor for OpcUaServerService.
/// @param name Name of the service.
OpcUaServerService::OpcUaServerService(const std::string& name)
    : NamedService(name) {
    // Initialize the change observer for the tree synchronization.
    m_observer = std::make_shared<TreeObserver>(*this);
}

OpcUaServerService::~OpcUaServerService() {
    // Ensure the server is stopped and resources are released.
    stop();
}

std::shared_ptr<OpcUaServerService> OpcUaServerService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    // Enabler struct to allow make_shared with a protected constructor.
    struct make_shared_enabler : public OpcUaServerService {
        /// @brief Helper constructor.
        explicit make_shared_enabler(const std::string& n) : OpcUaServerService(n) {}
    };
    // Create the service instance.
    std::shared_ptr<OpcUaServerService> self = std::make_shared<make_shared_enabler>(name);
    // Setup reflexive reference and hierarchy.
    self->setSelf(self);
    if (parent != nullptr) {
        self->setParent(parent);
    }
    
    // Bind the "run" method to execute the OPC UA iterate cycle.
    std::weak_ptr<OpcUaServerService> weakSelf = self;
    NamedMethod::create("run", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        if (std::shared_ptr<OpcUaServerService> s = weakSelf.lock()) {
            if (s->m_server) {
                // Execute one iteration of the server main loop.
                UA_Server_run_iterate(s->m_server, false);
            }
        }
        return nullptr;
    }, self);

    return self;
}

void OpcUaServerService::setPort(uint16_t port) {
    // Configure the listener port.
    m_port = port;
}

void OpcUaServerService::setRootObject(std::shared_ptr<NamedObject> root) {
    // Define the subtree to be exposed via OPC UA.
    m_rootObject = root;
}

void OpcUaServerService::start() {
    // Guard against multiple start calls.
    if (isRunning()) return;

    // Bootstrap the open62541 server instance.
    initializeOpcUa();
    // High frequency cycle for responsive network handling.
    setCycleTime(std::chrono::milliseconds(1));
    NamedService::start();
}

void OpcUaServerService::stop() {
    // Guard against multiple stop calls.
    if (!isRunning()) return;

    // Shut down the background execution thread.
    NamedService::stop();

    if (m_server != nullptr) {
        // Graceful server shutdown.
        UA_Server_run_shutdown(m_server);
        UA_Server_delete(m_server);
        m_server = nullptr;
    }
}


std::string OpcUaServerService::getType() const {
    // Identify the service class name.
    return "OpcUaServerService";
}

void OpcUaServerService::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    // Pass identity configuration to the security manager.
    m_securityManager.loadCertificate(certPath, keyPath);
}

void OpcUaServerService::loadTrustList(const std::vector<std::string>& trustListPaths) {
    // Pass trust configuration to the security manager.
    m_securityManager.loadTrustList(trustListPaths);
}

void OpcUaServerService::addUser(const std::string& username, const std::string& password) {
    // Store credentials for identity providers.
    m_users[username] = password;
}

void OpcUaServerService::setAllowAnonymous(bool allow) {
    // Control anonymous access permissions.
    m_allowAnonymous = allow;
}

void OpcUaServerService::initializeOpcUa() {
    // Instantiate the server object.
    m_server = UA_Server_new();
    UA_ServerConfig* config = UA_Server_getConfig(m_server);
    // Apply basic networking configuration.
    UA_ServerConfig_setMinimal(config, m_port, nullptr);
    
    // Configure encryption and secure channels.
    m_securityManager.configureServer(m_server, config);

    // Setup the identity provider if authentication is required.
    if (!m_users.empty() || !m_allowAnonymous) {
        size_t userCount = m_users.size();
        
        /// @brief Temporary vector holding user login details.
        std::vector<UA_UsernamePasswordLogin> logins;
        logins.resize(userCount);
        
        size_t i = 0;
        for (const std::pair<const std::string, std::string>& pair : m_users) {
            const std::string& u = pair.first;
            const std::string& p = pair.second;
            logins[i].username = UA_String_fromStdString(u);
            logins[i].password = UA_ByteString_fromStdString(p);
            i++;
        }
        // Initialize default access control plugin.
        UA_AccessControl_default(config, m_allowAnonymous, nullptr, userCount, logins.data());

        // Clear temporary strings after they have been copied by the plugin.
        for (size_t j = 0; j < userCount; ++j) {
            UA_String_clear(&logins[j].username);
            UA_ByteString_clear(&logins[j].password);
        }
    }

    // Determine the root of the address space.
    std::shared_ptr<NamedObject> root = m_rootObject ? m_rootObject : getSelf();
    // Recursively map the tree starting from the Objects folder.
    mapObject(root, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    // Prepare the server for the main loop.
    UA_Server_run_startup(m_server);
}


void OpcUaServerService::onWrite(UA_Server *server, const UA_NodeId *sessionId,
                                 void *sessionContext, const UA_NodeId *nodeId,
                                 void *nodeContext, const UA_NumericRange *range,
                                 const UA_DataValue *data) {
    (void)server; (void)sessionId; (void)sessionContext; (void)nodeId; (void)range;
    // Update the Quasar tree when an OPC UA client writes to a variable.
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
    
    // [CS-0010.15], [CS-0050.2] Check methodContext nullness explicitly.
    if (methodContext == nullptr) return UA_STATUSCODE_BADINTERNALERROR;

    NamedMethod* method = static_cast<NamedMethod*>(methodContext);
    
    // [CS-0050.2] Deserialise incoming arguments if input is provided.
    std::shared_ptr<NamedObject> args = nullptr;
    if (inputSize > 0 && input != nullptr) {
        // [CS-0010.14] Validate input argument types to avoid mismatches.
        if (input[0].type != &UA_TYPES[UA_TYPES_STRING]) return UA_STATUSCODE_BADTYPEMISMATCH;
        
        // Extract string data safely.
        UA_String uas = *(UA_String*)input[0].data;
        std::string jsonArgs;
        if (uas.data != nullptr && uas.length > 0) {
            jsonArgs = std::string((char*)uas.data, uas.length);
        }
        
        // Parse non-empty parameters.
        if (!jsonArgs.empty() && jsonArgs != "null") {
            args = serialization::fromJson(jsonArgs);
        }
    }

    // Execute logic and serialize result.
    std::shared_ptr<NamedObject> resObj = method->execute(args);
    std::string jsonRes = serialization::toJson(resObj);
    
    // [CS-0050.2] Return the response as an OPC UA string if output is expected.
    if (outputSize > 0 && output != nullptr) {
        UA_String uares = UA_STRING_ALLOC(jsonRes.c_str());
        UA_Variant_setScalarCopy(output, &uares, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&uares);
    }

    return UA_STATUSCODE_GOOD;
}

void OpcUaServerService::mapObject(std::shared_ptr<NamedObject> obj, UA_NodeId parentNodeId) {
    // Validate inputs.
    if (!m_server || !obj) return;

    // Fetch object identity and type.
    std::string objName = obj->getName();
    std::string type = obj->getType();
    // Allocate OPC UA names.
    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(1, objName.c_str());
    UA_LocalizedText displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", objName.c_str());

    UA_NodeId newNodeId;

    // Branching logic based on Quasar object type.
    if (type == "NamedMethod" || type == "NamedLuaMethod") {
        UA_MethodAttributes mAttr = UA_MethodAttributes_default;
        mAttr.displayName = displayName;
        mAttr.executable = true;
        mAttr.userExecutable = true;
        
        // Define standard JSON input argument.
        UA_Argument inputArgument;
        UA_Argument_init(&inputArgument);
        inputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "JSON Arguments");
        inputArgument.name = UA_STRING_ALLOC("Args");
        inputArgument.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
        inputArgument.valueRank = UA_VALUERANK_SCALAR;

        // Define standard JSON output argument.
        UA_Argument outputArgument;
        UA_Argument_init(&outputArgument);
        outputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "JSON Result");
        outputArgument.name = UA_STRING_ALLOC("Result");
        outputArgument.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
        outputArgument.valueRank = UA_VALUERANK_SCALAR;


        // Add the method node to the server address space.
        UA_Server_addMethodNode(m_server, UA_NODEID_NULL, parentNodeId,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                browseName, mAttr, &onMethodCall,
                                1, &inputArgument, 1, &outputArgument,
                                obj.get(), &newNodeId);
        
        // Cleanup temporary argument memory.
        UA_String_clear(&inputArgument.name);
        UA_LocalizedText_clear(&inputArgument.description);
        UA_String_clear(&outputArgument.name);
        UA_LocalizedText_clear(&outputArgument.description);
    } else if (type == "NamedInteger" || type == "NamedBoolean" || type == "NamedString" || type == "NamedFloatingPoint") {
        UA_VariableAttributes vAttr = UA_VariableAttributes_default;
        vAttr.displayName = displayName;
        // FIX: Store description to clear it later and avoid memory leak.
        vAttr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", type.c_str());
        vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        
        // Synchronize initial value.
        UA_Variant value = toUaVariant(obj);
        vAttr.value = value;
        
        // Create the variable node.
        UA_Server_addVariableNode(m_server, UA_NODEID_NULL, parentNodeId,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                  browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                  vAttr, obj.get(), &newNodeId);
        
        // Setup data-change callback for reverse sync (UA -> Quasar).
        UA_ValueCallback callback;
        callback.onRead = nullptr;
        callback.onWrite = onWrite;
        UA_Server_setVariableNode_valueCallback(m_server, newNodeId, callback);
        
        // Cleanup temporary variable memory.
        UA_Variant_clear(&value);
        // FIX: Clear the description to resolve the definitely lost memory leak.
        UA_LocalizedText_clear(&vAttr.description);
    } else {
        // Default to a simple folder-like object node.
        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName = displayName;
        UA_Server_addObjectNode(m_server, UA_NODEID_NULL, parentNodeId,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, obj.get(), &newNodeId);
    }

    // Cleanup browse and display names.
    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&displayName);

    // If node creation succeeded, track it for reactive updates.
    if (!UA_NodeId_isNull(&newNodeId)) {
        m_objectToNodeMap[obj] = newNodeId;
        obj->subscribe(m_observer);
    }


    // Recurse into children.
    for (std::shared_ptr<NamedObject> const& child : obj->getChildren()) {
        mapObject(child, newNodeId);
    }
}

void OpcUaServerService::handleObjectChanged(std::shared_ptr<named::NamedObject> obj) {
    // Guard against invalid state.
    if (!m_server || !obj) return;
    
    // Find the corresponding node in the OPC UA address space.
    std::map<std::shared_ptr<named::NamedObject>, UA_NodeId>::const_iterator it = m_objectToNodeMap.find(obj);
    if (it != m_objectToNodeMap.end()) {
        // Fetch the new value and write it to the server.
        UA_Variant value = toUaVariant(obj);
        if (!UA_Variant_isEmpty(&value)) {
            UA_Server_writeValue(m_server, it->second, value);
        }
        UA_Variant_clear(&value);
    }
}

} // namespace quasar::opcua
