/**
 * @file OpcUaClientService.cpp
 * @brief Implements OpcUaClientService connection lifecycle, address-space mirroring, and task dispatch.
 *
 * @details Connects an open62541 `UA_Client` to a remote OPC UA server, mirrors objects,
 * scalar variables, and methods into a local `NamedObject` tree, handles subscription
 * data-change callbacks, and executes remote method calls asynchronously via `enqueueTask`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200, FE-0220
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6
 * @compliance Preserves all runtime logic while documenting local helpers, structs, control
 * structures, and multi-line bodies per CS-0090 and CS-0010.44.
 */

// Client service header, variant helpers, and Quasar named/constant dependencies.
#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/UA_Common.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/coretypes/Constants.hpp"
// open62541 client runtime headers and standard exception/stream utilities.
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include <open62541/client_highlevel.h>
#include <stdexcept>
#include <sstream>

/**
 * @brief Quasar OPC UA integration namespace.
 *
 * @details Contains `OpcUaClientService` implementation and internal name-sanitization helpers.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Scopes client service implementation symbols inside `quasar::opcua`.
 */
namespace quasar::opcua {

using namespace named;

OpcUaClientService::OpcUaClientService(const std::string& name)
    : NamedService(name) {
}

OpcUaClientService::~OpcUaClientService() {
    stop();
}

std::shared_ptr<OpcUaClientService> OpcUaClientService::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    /**
     * @struct make_shared_enabler
     * @brief Local helper subclass exposing protected `OpcUaClientService` constructor to `std::make_shared`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2
     * @compliance Enables single-allocation `std::make_shared` construction without exposing raw constructor.
     */
    struct make_shared_enabler : public OpcUaClientService {
        /**
         * @brief Constructs the enabler forwarding `n` to protected `OpcUaClientService`.
         * @details Exposes protected `OpcUaClientService(const std::string&)` to `std::make_shared`.
         * @task TSK-20261113-003, TSK-20260311-005
         * @feature FE-0200
         * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.9
         * @compliance Explicit single-argument forwarding constructor.
         * @param[in] n
         *   Justification: Forwards service node name to base `OpcUaClientService`.
         *   Usage: Passed directly to base constructor initializer list.
         *   Consumed: No (read-only const reference).
         *   Prerequisites: Non-empty node name.
         * @pre !n.empty()
         */
        explicit make_shared_enabler(const std::string& n) : OpcUaClientService(n) {}
    };
    // Instantiate service via std::make_shared and bind reflexive self pointer.
    std::shared_ptr<OpcUaClientService> self = std::make_shared<make_shared_enabler>(name);
    self->setSelf(self);
    // Attach service to caller-supplied parent container when provided.
    if (parent) {
        self->setParent(parent);
    }
    
    // Register periodic "run" NamedMethod using weak_ptr to avoid ownership cycle.
    std::weak_ptr<OpcUaClientService> weakSelf = self;
    NamedMethod::create("run", [weakSelf](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        // Lock service weakly and drain queued tasks + open62541 client loop if connected.
        if (std::shared_ptr<OpcUaClientService> s = weakSelf.lock()) {
            // Run open62541 iteration only when client connection handle is active.
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
    // Guard against redundant starts when service is already active.
    if (isRunning()) return;

    // Connect client, mirror remote tree, and launch 10 ms service cycle.
    initializeClient();
    setCycleTime(std::chrono::milliseconds(10));
    NamedService::start();
}

void OpcUaClientService::stop() {
    // Return early if service is not running.
    if (!isRunning()) return;

    // Stop service worker thread prior to tearing down UA_Client handle.
    NamedService::stop();

    // Disconnect and free open62541 client instance if allocated.
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
    // Swap pending queue under mutex lock so tasks run without holding m_queueMutex.
    std::queue<std::function<void(UA_Client*)>> currentTasks;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::swap(currentTasks, m_tasks);
    }
    
    // Drain bounded snapshot queue sequentially on the service thread [CS-0010.37].
    while (!currentTasks.empty()) {
        currentTasks.front()(m_client);
        currentTasks.pop();
    }
}

/**
 * @brief Sanitizes an OPC UA browse name into a valid Quasar `NamedObject` identifier.
 *
 * @details Replaces non-alphanumeric characters (other than `'_'`) with `'_'`, defaults empty
 * inputs to `"unnamed"`, and prepends `'_'` if the first character is a digit.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Bounded iteration over `name` characters and deterministic identifier prefixing.
 *
 * @param[in] name
 *   Justification: Supplies the raw OPC UA `BrowseName` string retrieved from a remote node.
 *   Usage: Iterated character-by-character to construct a safe Quasar node name.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Any `std::string` (empty string safely yields `"unnamed"`).
 *
 * @pre true (empty strings are handled explicitly).
 * @post Returned string is non-empty, starts with a non-digit character, and contains only `[A-Za-z0-9_]`.
 * @return std::string
 *   Justification: Supplies a Lua/Quasar-safe identifier for mirrored child nodes.
 *   Usage: Passed directly as the child name to `NamedObject::create` / `NamedMethod::create`.
 *   Consumed: Value returned by move/NRVO; caller owns string.
 *   Prerequisites: None.
 *   Limitations: Distinct remote names differing only in punctuation may map to identical sanitized names.
 *   Lifecycle: Independent caller-owned `std::string`.
 *   Modification / Deletion: Caller may freely mutate or discard returned string.
 *   Specific Values: `"unnamed"` if input is empty; otherwise sanitized `[A-Za-z_][A-Za-z0-9_]*`.
 *   Error Behavior: Propagates `std::bad_alloc` only if string growth fails under OOM.
 * @throws std::bad_alloc [Resource / Allocation] Thrown if output string allocation fails.
 * @error
 *   - Validation / Contract (empty `name`): Returns `"unnamed"` immediately.
 *   - Resource / Allocation (OOM during string build): Propagates `std::bad_alloc`.
 * @containment
 *   Approach: Bounded single pass over input characters with explicit leading-digit prefixing.
 *   Limitations: Does not disambiguate name collisions created by replacing special characters with `'_'`.
 */
static std::string sanitizeName(const std::string& name) {
    // Handle empty remote browse names cleanly.
    if (name.empty()) return "unnamed";
    std::string res;
    // Scan every character bounded by name.size() [CS-0010.37].
    for (char c : name) {
        // Keep ASCII alphanumeric/underscore characters and replace all others with '_'.
        if (std::isalnum(c) || c == '_') {
            res += c;
        } else {
            // Normalize punctuation/whitespace to underscore.
            res += '_';
        }
    }
    // Prepend underscore when identifier begins with a digit for scripting compatibility.
    if (std::isdigit(res[0])) {
        res = "_" + res;
    }
    return res;
}

void OpcUaClientService::onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                                      UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    (void)client; (void)subId; (void)subContext; (void)monId;
    // Update local mirrored NamedObject only when context and payload value are valid [CS-0010.15].
    if (monContext && value->hasValue) {
        NamedObject* obj = static_cast<NamedObject*>(monContext);
        fromUaVariant(&value->value, obj->getSelf());
    }
}

void OpcUaClientService::initializeClient() {
    // Allocate and initialize open62541 client with default policy configuration.
    m_client = UA_Client_new();
    UA_ClientConfig* config = UA_Client_getConfig(m_client);
    UA_ClientConfig_setDefault(config);
    config->allowNonePolicyPassword = true;

    // Configure security via the manager
    m_securityManager.configureClient(m_client);

    // Configure Credentials when username is non-empty.
    if (!m_username.empty()) {
        UA_ClientConfig_setAuthenticationUsername(config, m_username.c_str(), m_password.c_str());
    }

    // Establish TCP/OPC UA session; clean up and throw on connection failure.
    UA_StatusCode retval = UA_Client_connect(m_client, m_url.c_str());
    // Delete client handle prior to throwing if connection failed [CS-0010.20].
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(m_client);
        m_client = nullptr;
        throw std::runtime_error("Failed to connect to OPC UA server: " + m_url);
    }

    // Create a single subscription for all variables
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(m_client, request,
                                                                            nullptr, nullptr, nullptr);
    // Record subscription ID when server accepted subscription creation.
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        m_subscriptionId = response.subscriptionId;
    }

    // Recursively mirror remote ObjectsFolder subtree into local NamedObject tree.
    browseAndMirror(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), getSelf());
}

void OpcUaClientService::browseAndMirror(UA_NodeId remoteNodeId, std::shared_ptr<NamedObject> localParent) {
    // Build single-node browse request covering all forward references.
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    // Allocate browse description array via open62541 helper.
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    UA_NodeId_copy(&remoteNodeId, &bReq.nodesToBrowse[0].nodeId);
    bReq.nodesToBrowse[0].referenceTypeId = UA_NODEID_NULL;
    // Include reference subtypes and full browse metadata mask.
    bReq.nodesToBrowse[0].includeSubtypes = true;
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

    // Execute synchronous browse against connected OPC UA server.
    UA_BrowseResponse bRes = UA_Client_Service_browse(m_client, bReq);
    
    // Iterate over browse result sets bounded by bRes.resultsSize [CS-0010.37].
    for (size_t i = 0; i < bRes.resultsSize; ++i) {
        // Iterate over reference descriptions bounded by referencesSize [CS-0010.37].
        for (size_t j = 0; j < bRes.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &bRes.results[i].references[j];
            
            // Copy target NodeId for inspection and child recursion.
            UA_NodeId targetId;
            UA_NodeId_init(&targetId);
            UA_NodeId_copy(&ref->nodeId.nodeId, &targetId);

            // Convert remote browse name to sanitized Quasar node identifier.
            std::string rawName((char*)ref->browseName.name.data, ref->browseName.name.length);
            printf("[C++] Found ref: %s (ns=%d, class=%d)\n", rawName.c_str(), targetId.namespaceIndex, ref->nodeClass);
            std::string name = sanitizeName(rawName);
            
            // Skip standard OPC UA namespace-0 root system nodes to avoid mirroring server metadata.
            if (targetId.namespaceIndex == 0 && 
                (rawName == "Server" || rawName == "Types" || rawName == "Views")) {
                UA_NodeId_clear(&targetId);
                continue;
            }

            std::shared_ptr<NamedObject> localObj;
            
            // Create either a remote-calling NamedMethod proxy or a mirrored data/object node.
            if (ref->nodeClass == UA_NODECLASS_METHOD) {
                // Copy method and parent object NodeIds for capture inside RPC lambda.
                UA_NodeId methodId;
                UA_NodeId_copy(&targetId, &methodId);
                UA_NodeId objectId;
                UA_NodeId_copy(&remoteNodeId, &objectId);
                
                // Capture weak service pointer to marshal remote calls onto service thread.
                std::weak_ptr<OpcUaClientService> weakSvc = std::dynamic_pointer_cast<OpcUaClientService>(getSelf());
                localObj = NamedMethod::create(name, [weakSvc, methodId, objectId](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
                    (void)owner;
                    std::shared_ptr<OpcUaClientService> s = weakSvc.lock();
                    // Abort cleanly if client service expired or is no longer running.
                    if (!s || !s->isRunning()) return nullptr;
                    
                    // Build UA_Client_call closure queued onto the service thread.
                    std::function<std::shared_ptr<NamedObject>(UA_Client*)> task = [methodId, objectId, args](UA_Client* client) -> std::shared_ptr<NamedObject> {
                        UA_Variant input;
                        UA_Variant_init(&input);
                        // Serialize Quasar arguments to JSON string variant.
                        std::string jsonArgs = serialization::toJson(args);
                        UA_String uas = UA_STRING_ALLOC(jsonArgs.c_str());
                        UA_Variant_setScalarCopy(&input, &uas, &UA_TYPES[UA_TYPES_STRING]);
                        UA_String_clear(&uas);
                        
                        // Invoke synchronous UA_Client_call on the client thread.
                        size_t outputSize = 0;
                        UA_Variant *output = nullptr;
                        UA_StatusCode retval = UA_Client_call(client, objectId, methodId, 1, &input, &outputSize, &output);
                        UA_Variant_clear(&input);
                        
                        std::shared_ptr<NamedObject> res = nullptr;
                        // Deserialize JSON response string when call succeeded and returned a string variant.
                        if (retval == UA_STATUSCODE_GOOD && outputSize > 0 && output[0].type == &UA_TYPES[UA_TYPES_STRING]) {
                            UA_String uares = *(UA_String*)output[0].data;
                            std::string jsonRes((char*)uares.data, uares.length);
                            // Skip deserialization when remote method returned JSON "null".
                            if (jsonRes != "null") {
                                res = serialization::fromJson(jsonRes);
                            }
                        }
                        // Free output variant array returned by UA_Client_call.
                        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                        
                        // [FE-0130.4.3] Ensure we return a valid NamedObject for Lua, even if method had no output
                        if (!res) {
                            res = NamedObject::create("void");
                        }
                        return res;
                    };

                    // Enqueue RPC task and wait with bounded timeout [CS-0010.26].
                    try {
                        std::future<std::shared_ptr<NamedObject>> future = s->enqueueTask<std::shared_ptr<NamedObject>>(task);
                        // Return deserialized object on ready, or "timeout" marker on deadline expiry.
                        if (future.wait_for(quasar::coretypes::DEFAULT_OPCUA_METHOD_TIMEOUT) == std::future_status::ready) {
                            std::shared_ptr<NamedObject> r = future.get();
                            return r ? r : NamedObject::create("void");
                        } else {
                            // Timeout expired before service thread completed RPC call.
                            return NamedObject::create("timeout");
                        }
                    } catch (...) {
                        // Return sentinel "error" NamedObject if RPC queuing or execution threw.
                        return NamedObject::create("error");
                    }
                }, localParent);
            } else {
                // Read current attribute value from remote variable/object node.
                UA_Variant value;
                UA_Variant_init(&value);
                UA_Client_readValueAttribute(m_client, targetId, &value);
                
                // Instantiate corresponding typed Quasar NamedObject matching UA scalar type.
                if (value.type == &UA_TYPES[UA_TYPES_INT32]) {
                    localObj = NamedInteger<int32_t>::create(name, *(UA_Int32*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_INT64]) {
                    // Instantiate 64-bit NamedInteger mirror.
                    localObj = NamedInteger<int64_t>::create(name, *(UA_Int64*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                    // Instantiate NamedBoolean mirror.
                    localObj = NamedBoolean::create(name, *(UA_Boolean*)value.data);
                } else if (value.type == &UA_TYPES[UA_TYPES_STRING]) {
                    // Instantiate NamedString mirror from UA_String slice.
                    UA_String uas = *(UA_String*)value.data;
                    localObj = NamedString::create(name, std::string((char*)uas.data, uas.length));
                } else if (value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
                    // Instantiate double-precision NamedFloatingPoint mirror.
                    localObj = NamedFloatingPoint<double>::create(name, *(UA_Double*)value.data);
                } else {
                    // Default non-scalar or unsupported types to generic NamedObject container.
                    localObj = NamedObject::create(name);
                }
                // Clear temporary variant and attach mirrored object under localParent.
                UA_Variant_clear(&value);
                localObj->setParent(localParent);
            }
            
            // Subscribe to data-change notifications for variable nodes when subscription is active.
            if (ref->nodeClass == UA_NODECLASS_VARIABLE) {
                // Register monitored item only if subscription creation succeeded.
                if (m_subscriptionId != 0) {
                    UA_MonitoredItemCreateRequest monRequest =
                        UA_MonitoredItemCreateRequest_default(targetId);
                    // Register onDataChange callback with raw mirrored object pointer as context.
                    UA_Client_MonitoredItems_createDataChange(m_client, m_subscriptionId,
                                                              UA_TIMESTAMPSTORETURN_BOTH, monRequest,
                                                              localObj.get(), onDataChange, nullptr);
                }
            }
            
            // Recurse into child nodes for Object and Variable containers.
            if (ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE) {
                // Recursively mirror children if they are not system objects
                if (!(targetId.namespaceIndex == 0 && targetId.identifierType == UA_NODEIDTYPE_NUMERIC && targetId.identifier.numeric < 100)) {
                    browseAndMirror(targetId, localObj);
                }
            }
            // Clear temporary target NodeId before advancing to next reference.
            UA_NodeId_clear(&targetId);
        }
    }
    
    // Free open62541 browse request and response buffers.
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bRes);
}

} // namespace quasar::opcua
