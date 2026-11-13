/**
 * @file OpcUaServerService.hpp
 * @brief Declares OpcUaServerService for exposing a Quasar NamedObject hierarchy as an OPC UA server.
 *
 * @details Defines a `named::NamedService` that maps `NamedObject` trees (objects, scalar
 * variables, and `NamedMethod` nodes) into an open62541 `UA_Server` address space, keeps
 * variable values synchronized bidirectionally, and enforces X.509/username authentication.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200, FE-0220, FE-0240
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8
 * @compliance Documents all public, protected, and private server service entities with
 * complete parameter, return, and error-containment contracts.
 */

#ifndef QUASAR_OPCUA_OPCUASERVERSERVICE_HPP
#define QUASAR_OPCUA_OPCUASERVERSERVICE_HPP

// Quasar NamedService base and OPC UA security manager dependencies.
#include "quasar/named/NamedService.hpp"
#include "quasar/opcua/OpcUaSecurityManager.hpp"
// open62541 server APIs, logging plugin, and standard library containers.
#include <open62541/server.h>
#include <open62541/plugin/log_stdout.h>
#include <memory>
#include <map>
#include <vector>
// Standard string support.
#include <string>

/**
 * @brief Quasar OPC UA integration namespace.
 *
 * @details Encapsulates OPC UA server, client, security, and variant marshaling utilities.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Houses `OpcUaServerService` inside `quasar::opcua`.
 */
namespace quasar::opcua {

/**
 * @class OpcUaServerService
 * @brief NamedService that exposes a NamedObject tree via OPC UA.
 *
 * @details Fulfills [TSK-20260311-005.2] OPC UA Server as NamedService and [FE-0200.2] OPC UA Server Service.
 * Maps a Quasar `NamedObject` subtree under the OPC UA `ObjectsFolder`, tracks local tree updates
 * via `TreeObserver`, synchronizes OPC UA client writes back to Quasar nodes, and exposes
 * `NamedMethod` children as callable OPC UA methods using JSON argument/result serialization.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200, FE-0220, FE-0240
 * @constraint CS-0010.6, CS-0010.21, CS-0060.5, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant When running, `m_server != nullptr` and `m_objectToNodeMap` tracks mirrored nodes subscribed to `m_observer`.
 * @compliance Manages server startup/shutdown deterministically via `NamedService` lifecycle and `~OpcUaServerService()`.
 */
class OpcUaServerService : public named::NamedService {
public:
    /**
     * @brief Factory method creating a managed OpcUaServerService instance.
     *
     * @details Instantiates the service using a `make_shared_enabler`, binds `setSelf`,
     * optionally attaches to `parent`, and registers the `"run"` iteration method.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Allocates via `std::make_shared` (no raw `new`) and avoids reference cycles with `std::weak_ptr`.
     *
     * @param[in] name
     *   Justification: Assigns the unique `NamedObject` name for this server service node.
     *   Usage: Passed to protected constructor `OpcUaServerService(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty valid object name.
     * @param[in] parent
     *   Justification: Optionally attaches the service to a parent `NamedObject` container.
     *   Usage: Passed to `self->setParent(parent)` when non-null.
     *   Consumed: No (shared_ptr copied into parent linkage if non-null).
     *   Prerequisites: May be `nullptr`; otherwise valid `NamedObject`.
     *
     * @pre !name.empty()
     * @post Returned `shared_ptr` is non-null, self-bound, and contains a `"run"` `NamedMethod`.
     * @return std::shared_ptr<OpcUaServerService>
     *   Justification: Provides shared ownership compatible with Quasar's reflexive object tree.
     *   Usage: Caller configures port/root/security and invokes `start()`.
     *   Consumed: Ownership shared via `std::shared_ptr`.
     *   Prerequisites: Non-null on normal return.
     *   Limitations: Port and security settings must be configured before `start()`.
     *   Lifecycle: Valid until all external and parent `shared_ptr` references are released.
     *   Modification / Deletion: Caller may mutate configuration via public setters and release `shared_ptr` to destroy.
     *   Specific Values: Non-null `std::shared_ptr<OpcUaServerService>`.
     *   Error Behavior: Never returns `nullptr`; throws on allocation or parent attachment failure.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared` fails under OOM.
     * @throws std::exception [Validation / Contract] Propagated if `setParent` or `NamedMethod::create` throws.
     * @error
     *   - Resource / Allocation (heap exhaustion): Propagates `std::bad_alloc` with no state leaked.
     *   - Validation / Contract (invalid parent or duplicate child name): Propagates exception cleanly.
     * @containment
     *   Approach: `std::make_shared` and `std::weak_ptr` prevent memory leaks or retain cycles on failure.
     *   Limitations: Any failure in `setParent` aborts creation without returning an instance.
     */
    static std::shared_ptr<OpcUaServerService> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor stopping the OPC UA server and freeing open62541 resources.
     *
     * @details Invokes `stop()` to halt the service loop, run server shutdown, and delete `m_server`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.21, CS-0060.5, CS-0090.1, CS-0090.2
     * @exposed
     * @compliance Guarantees RAII shutdown of open62541 server handle on destruction.
     */
    virtual ~OpcUaServerService();

    /**
     * @brief Sets the TCP port for the OPC UA server.
     *
     * @details Updates `m_port` used during `initializeOpcUa()` when configuring the listening socket.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Explicit port assignment prior to server socket binding.
     *
     * @param[in] port
     *   Justification: Selects the TCP port on which the OPC UA server listens.
     *   Usage: Stored in `m_port` and passed to `UA_ServerConfig_setMinimal`.
     *   Consumed: Yes (copied scalar `uint16_t`).
     *   Prerequisites: Valid TCP port number (e.g., `4840` or ephemeral `0`).
     *
     * @pre !isRunning()
     * @post `m_port == port`.
     * @error
     *   - Validation / Contract (called after `start()`): Does not rebind an already running server.
     * @containment
     *   Approach: Scalar assignment stored prior to `initializeOpcUa()`.
     *   Limitations: Does not check port availability until `UA_Server_run_startup` runs in `start()`.
     */
    void setPort(uint16_t port);

    /**
     * @brief Sets the root object to expose.
     *
     * @details If not set, the service itself is the root mapped under the OPC UA `ObjectsFolder`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Shares ownership of the exposed root subtree safely via `std::shared_ptr`.
     *
     * @param[in] root
     *   Justification: Selects an arbitrary Quasar subtree to publish over OPC UA.
     *   Usage: Stored in `m_rootObject` and traversed by `mapObject()` on `start()`.
     *   Consumed: No (shared_ptr copied into `m_rootObject`).
     *   Prerequisites: May be `nullptr` (falls back to `getSelf()`); otherwise valid subtree root.
     *
     * @pre !isRunning()
     * @post `m_rootObject == root`.
     * @error
     *   - Validation / Contract (called while running): Existing mapped address space is not rebuilt until restart.
     * @containment
     *   Approach: Safely retains shared ownership of `root` in `m_rootObject`.
     *   Limitations: Must be invoked before `start()` to take effect in `initializeOpcUa()`.
     */
    void setRootObject(std::shared_ptr<named::NamedObject> root);

    /**
     * @brief Starts the service.
     *
     * @details Checks `isRunning()`, calls `initializeOpcUa()`, configures a 1 ms cycle time,
     * and launches the background thread via `NamedService::start()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards duplicate starts and initializes open62541 address space before spawning thread.
     *
     * @pre `!isRunning()` and configured TCP port is available.
     * @post Server is initialized, address space mapped, and `isRunning() == true`.
     * @throws std::exception [Resource / Fatal] Propagates exceptions from `initializeOpcUa()` or `NamedService::start()`.
     * @error
     *   - Validation / Contract (already running): Returns early without side effects.
     *   - Resource / Fatal (thread creation or mapping failure): Propagates exception to caller.
     * @containment
     *   Approach: Idempotency check `if (isRunning()) return;` prevents duplicate server instances.
     *   Limitations: `UA_Server_run_startup` status is not explicitly checked to throw on port bind conflict.
     */
    void start() override;

    /**
     * @brief Stops the service.
     *
     * @details Returns early if not running; otherwise stops `NamedService`, runs
     * `UA_Server_run_shutdown`, deletes `m_server`, and clears `m_server` to `nullptr`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Joins worker thread before calling `UA_Server_run_shutdown` and `UA_Server_delete`.
     *
     * @post `!isRunning()` and `m_server == nullptr`.
     * @throws std::exception [Resource / Fatal] Propagates if `NamedService::stop()` throws.
     * @error
     *   - Validation / Contract (already stopped): Returns early cleanly.
     *   - Resource / Fatal (thread stop failure): Propagates exception prior to server deletion.
     * @containment
     *   Approach: Sequential `NamedService::stop()` followed by null-checked `UA_Server_delete` and nulling `m_server`.
     *   Limitations: Observer subscriptions registered on mapped `NamedObject`s are retained via `weak_ptr`/observer handle until objects expire.
     */
    void stop() override;

    /**
     * @brief Gets the type name.
     *
     * @details Overrides `NamedObject::getType()` with `"OpcUaServerService"`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Returns deterministic class name for Quasar reflexive dispatch.
     *
     * @post Returned string equals `"OpcUaServerService"`.
     * @return std::string
     *   Justification: Identifies the service type in Quasar's reflexive hierarchy.
     *   Usage: Caller compares returned string against `"OpcUaServerService"`.
     *   Consumed: Value returned by move/NRVO; caller owns string.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may freely mutate or discard returned string.
     *   Specific Values: Always `"OpcUaServerService"`.
     *   Error Behavior: May throw `std::bad_alloc` only under extreme OOM if SSO is unavailable.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Direct literal string return (zero manual allocation).
     *   Limitations: Not `noexcept` because base virtual method omits `noexcept`.
     */
    std::string getType() const override;

    /**
     * @brief Loads server certificate and private key.
     *
     * @details Forwards DER certificate and private key paths to `m_securityManager.loadCertificate`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates credential loading to `OpcUaSecurityManager`.
     *
     * @param[in] certPath
     *   Justification: Locates the server DER X.509 certificate file.
     *   Usage: Forwarded to `m_securityManager.loadCertificate`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty readable path to DER certificate.
     * @param[in] keyPath
     *   Justification: Locates the server DER private key file.
     *   Usage: Forwarded to `m_securityManager.loadCertificate`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty readable path to matching DER private key.
     *
     * @pre !certPath.empty() && !keyPath.empty()
     * @post `m_securityManager` stores loaded server identity when both files are valid.
     * @throws std::bad_alloc [Resource / Allocation] Propagates if internal string allocations fail.
     * @error
     *   - Hardware / Bus / I/O (missing or empty file): Ignored cleanly inside `m_securityManager`; prior credentials preserved.
     * @containment
     *   Approach: Delegates to `OpcUaSecurityManager::loadCertificate` strong-rollback implementation.
     *   Limitations: Void signature does not return `UA_StatusCode` failure to caller.
     */
    void loadCertificate(const std::string& certPath, const std::string& keyPath);

    /**
     * @brief Loads trust list for client certificate validation.
     *
     * @details Forwards trusted DER certificate paths to `m_securityManager.loadTrustList`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates client PKI trust-list ingestion to `OpcUaSecurityManager`.
     *
     * @param[in] trustListPaths
     *   Justification: Lists paths of trusted client/CA DER certificates.
     *   Usage: Forwarded to `m_securityManager.loadTrustList`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Vector of valid filesystem paths (may be empty).
     *
     * @pre true (empty list is valid and enables `AcceptAll` development mode).
     * @post Readable trust certificates are appended to `m_securityManager`.
     * @throws std::bad_alloc [Resource / Allocation] Propagates if trust vector allocation fails.
     * @error
     *   - Hardware / Bus / I/O (unreadable trust file): Skipped inside `m_securityManager`.
     * @containment
     *   Approach: Delegates to `OpcUaSecurityManager::loadTrustList`.
     *   Limitations: Void signature does not surface individual skipped files.
     */
    void loadTrustList(const std::vector<std::string>& trustListPaths);

    /**
     * @brief Adds a user for username/password authentication.
     *
     * @details Registers or overwrites `m_users[username] = password` for consumption during
     * `initializeOpcUa()` access-control initialization.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Stores user credentials explicitly prior to `UA_AccessControl_default` configuration.
     *
     * @param[in] username
     *   Justification: Identifies the client login username key.
     *   Usage: Used as key in `m_users` map.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty username string.
     * @param[in] password
     *   Justification: Supplies the password secret associated with `username`.
     *   Usage: Stored as mapped value in `m_users[username]`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Valid password string.
     *
     * @pre !username.empty()
     * @post `m_users[username] == password`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if map node or string insertion fails.
     * @error
     *   - Resource / Allocation (map insertion OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Standard `std::map` insertion prior to `start()`.
     *   Limitations: Must be called before `start()`; stores password plaintext in `m_users`.
     */
    void addUser(const std::string& username, const std::string& password);

    /**
     * @brief Sets whether anonymous login is allowed.
     *
     * @details Updates `m_allowAnonymous` consulted by `UA_AccessControl_default` in `initializeOpcUa()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Explicit boolean policy toggle for anonymous OPC UA sessions.
     *
     * @param[in] allow
     *   Justification: Controls whether unauthenticated anonymous OPC UA clients can connect.
     *   Usage: Assigned directly to `m_allowAnonymous`.
     *   Consumed: Yes (copied scalar boolean).
     *   Prerequisites: None (any `bool`).
     *
     * @pre !isRunning()
     * @post `m_allowAnonymous == allow`.
     * @error
     *   - Validation / Contract (called while running): Takes effect only on next `start()`.
     * @containment
     *   Approach: Direct scalar flag assignment prior to access-control plugin registration.
     *   Limitations: Does not disconnect existing anonymous sessions if changed at runtime.
     */
    void setAllowAnonymous(bool allow);

protected:
    /**
     * @brief Constructor initializing base NamedService and internal TreeObserver.
     *
     * @details Protected so instances are created exclusively via `OpcUaServerService::create()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Enforces `explicit` construction and initializes `m_observer` via `std::make_shared`.
     *
     * @param[in] name
     *   Justification: Supplies the Quasar `NamedObject` identifier for the server service.
     *   Usage: Forwarded to `NamedService(name)` base constructor.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     *
     * @pre !name.empty()
     * @post Base `NamedService` initialized and `m_observer != nullptr`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared<TreeObserver>` fails.
     * @error
     *   - Resource / Allocation (observer allocation OOM): Propagates cleanly.
     * @containment
     *   Approach: RAII `std::make_shared` initialization inside constructor body.
     *   Limitations: Reflexive `setSelf` must be completed by `create()`.
     */
    explicit OpcUaServerService(const std::string& name);

    /**
     * @brief Initializes the OPC UA server and maps the tree.
     *
     * @details Allocates `UA_Server`, applies minimal config on `m_port`, configures PKI via
     * `m_securityManager`, configures `UA_AccessControl_default` if users/restrictions exist,
     * maps `m_rootObject` (or `getSelf()`) recursively under `ObjectsFolder`, and calls
     * `UA_Server_run_startup`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200, FE-0220
     * @constraint CS-0010.34, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @compliance Frees temporary `UA_UsernamePasswordLogin` buffers after `UA_AccessControl_default` copies them.
     *
     * @pre `m_server == nullptr`
     * @post `m_server != nullptr`, address space mapped, and server started.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if temporary login vector allocation fails.
     * @error
     *   - Resource / Allocation (login vector OOM): Propagates before startup; open62541 temp strings cleared on normal flow.
     * @containment
     *   Approach: Clears all temporary `UA_String` and `UA_ByteString` login copies immediately after `UA_AccessControl_default`.
     *   Limitations: Does not delete `m_server` if `UA_Server_run_startup` returns a bad status code.
     */
    void initializeOpcUa();

    /**
     * @brief Main loop hook declaration for server execution.
     *
     * @details Declared in header for subclass/internal server-thread lifecycle hook; runtime
     * iteration is driven by the `"run"` `NamedMethod` registered in `create()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.8
     * @compliance Documented protected lifecycle hook declaration.
     *
     * @error
     *   - Validation / Contract (unused hook): Not invoked directly at runtime.
     * @containment
     *   Approach: Actual iteration executes through `NamedService` periodic `"run"` method.
     *   Limitations: Reserved declaration with no standalone `.cpp` body.
     */
    void runServer();

    /**
     * @brief Recursively maps a NamedObject tree to OPC UA nodes.
     *
     * @details Creates an OPC UA method node (`NamedMethod`/`NamedLuaMethod`), writable scalar
     * variable node (`NamedInteger`, `NamedBoolean`, `NamedString`, `NamedFloatingPoint`), or
     * base object node, subscribes `m_observer` for Quasar->OPC UA updates, and recurses on children.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.15, CS-0010.34, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Guards null `m_server`/`obj` and clears all temporary open62541 attributes/names/variants.
     *
     * @param[in] obj
     *   Justification: Supplies the Quasar `NamedObject` node to map into the OPC UA address space.
     *   Usage: Read for name, type, initial scalar value, and child list; subscribed to `m_observer`.
     *   Consumed: No (shared_ptr copied into `m_objectToNodeMap` key if node creation succeeds).
     *   Prerequisites: May be `nullptr` (early return); otherwise valid `NamedObject`.
     * @param[in] parentNodeId
     *   Justification: Identifies the parent OPC UA node under which the mapped node is attached.
     *   Usage: Passed to `UA_Server_add*Node` as the `HasComponent` parent.
     *   Consumed: Yes (passed by value `UA_NodeId`).
     *   Prerequisites: Valid existing `UA_NodeId` in `m_server`.
     *
     * @pre (m_server == nullptr || obj == nullptr) || !UA_NodeId_isNull(&parentNodeId)
     * @post `obj` and its descendants are mapped in `m_server` and tracked in `m_objectToNodeMap`.
     * @throws std::exception [Resource / Allocation] Propagates if `m_objectToNodeMap` insertion or child traversal throws.
     * @error
     *   - Validation / Contract (`m_server == nullptr` or `obj == nullptr`): Returns early safely.
     *   - Resource / Allocation (node creation fails leaving null `newNodeId`): Skips map insertion/subscription cleanly.
     * @containment
     *   Approach: Clears `UA_QualifiedName`, `UA_LocalizedText`, `UA_Argument` names/descriptions, and `UA_Variant` immediately after open62541 copies them.
     *   Limitations: Assumes Quasar tree is acyclic; `obj.get()` raw context pointer relies on `m_objectToNodeMap` keeping `shared_ptr` alive.
     */
    void mapObject(std::shared_ptr<named::NamedObject> obj, UA_NodeId parentNodeId);

    /**
     * @brief Internal callback for object changes propagating from Quasar to OPC UA.
     *
     * @details Looks up `obj` in `m_objectToNodeMap`, converts its current value via `toUaVariant`,
     * and writes non-empty variants into `m_server` via `UA_Server_writeValue`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.15, CS-0010.34, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Guards null `m_server`/`obj`, uses explicit iterator type, and clears `UA_Variant`.
     *
     * @param[in] obj
     *   Justification: Identifies the `NamedObject` whose value changed in the Quasar tree.
     *   Usage: Looked up in `m_objectToNodeMap` and converted to `UA_Variant`.
     *   Consumed: No (read-only shared_ptr copy).
     *   Prerequisites: May be `nullptr` (safe no-op); otherwise valid mapped `NamedObject`.
     *
     * @pre true (null `m_server` or `obj` handled safely).
     * @post Corresponding OPC UA variable node value matches `obj` when mapped and non-empty.
     * @throws std::exception [Validation / Contract] Propagates if `toUaVariant` throws.
     * @error
     *   - Validation / Contract (`m_server == nullptr`, `obj == nullptr`, unmapped `obj`, or empty variant): Safely ignored; temporary variant cleared.
     * @containment
     *   Approach: Null guards, map lookup check, and `UA_Variant_clear(&value)` on every exit path.
     *   Limitations: No mutex protects concurrent `UA_Server_writeValue` if `obj` mutates from non-service threads.
     */
    void handleObjectChanged(std::shared_ptr<named::NamedObject> obj);

private:
    /**
     * @brief Open62541 server instance handle owned by this service.
     * @details Created in `initializeOpcUa()`, iterated in `"run"`, and freed in `stop()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized to `nullptr` and cleaned up on `stop()`/destruction.
     */
    UA_Server* m_server{nullptr};

    /**
     * @brief Non-owning pointer to open62541 server configuration (reserved).
     * @details Initialized to `nullptr`; active configuration is queried directly from `m_server`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized to `nullptr`.
     */
    UA_ServerConfig* m_config{nullptr};

    /**
     * @brief TCP port number on which the OPC UA server listens.
     * @details Defaults to standard OPC UA port `4840` and configurable via `setPort()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized to `4840`.
     */
    uint16_t m_port{4840};

    /**
     * @brief Root Quasar `NamedObject` subtree exposed under OPC UA `ObjectsFolder`.
     * @details When null, `initializeOpcUa()` exposes `getSelf()` instead.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed shared pointer retaining exposed subtree lifetime.
     */
    std::shared_ptr<named::NamedObject> m_rootObject;

    /**
     * @brief Security manager owning server certificates, keys, and client trust list.
     * @details Configured prior to `start()` and applied during `initializeOpcUa()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2
     * @compliance RAII member automatically clears security buffers upon service destruction.
     */
    OpcUaSecurityManager m_securityManager;

    /**
     * @brief Allowed username-to-password authentication map.
     * @details Populated via `addUser()` and registered in `UA_AccessControl_default`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Standard `std::map` managing user credentials cleanly.
     */
    std::map<std::string, std::string> m_users;

    /**
     * @brief Flag controlling whether anonymous OPC UA client sessions are permitted.
     * @details Defaults to `true` and updated via `setAllowAnonymous()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized boolean flag.
     */
    bool m_allowAnonymous{true};

    /**
     * @brief Map linking exposed `NamedObject` instances to their OPC UA `UA_NodeId`s.
     * @details Populated in `mapObject()` and queried by `handleObjectChanged()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Retains `shared_ptr` keys so `nodeContext` raw pointers remain valid while mapped.
     */
    std::map<std::shared_ptr<named::NamedObject>, UA_NodeId> m_objectToNodeMap;
    
    /** @brief Forward declaration of internal observer helper tracking Quasar tree changes. */
    class TreeObserver;
    
    /**
     * @brief Shared observer instance subscribed to mapped `NamedObject` nodes.
     * @details Allocated in constructor and forwards `notify()` callbacks to `handleObjectChanged()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Owned via `std::shared_ptr<TreeObserver>`.
     */
    std::shared_ptr<TreeObserver> m_observer;

    /**
     * @brief Static open62541 value-write callback synchronizing client writes back to Quasar nodes.
     *
     * @details Casts `nodeContext` to `named::NamedObject*` and invokes `fromUaVariant` when
     * `nodeContext != nullptr && data->hasValue`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Guards `nodeContext` and `data->hasValue` before dereferencing or converting variant data.
     *
     * @param[in] server
     *   Justification: Required by open62541 `UA_ValueCallback::onWrite` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Valid `UA_Server*` provided by open62541.
     * @param[in] sessionId
     *   Justification: Required by open62541 callback signature identifying client session.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] sessionContext
     *   Justification: Required by open62541 callback signature for session user context.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] nodeId
     *   Justification: Required by open62541 callback signature identifying written node.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Valid `UA_NodeId*`.
     * @param[in,out] nodeContext
     *   Justification: Carries raw `NamedObject*` registered when creating the variable node.
     *   Usage: Cast to `NamedObject*` to resolve `getSelf()` and update value via `fromUaVariant`.
     *   Consumed: Mutated indirectly (target `NamedObject` value updated).
     *   Prerequisites: May be `nullptr` (guarded); otherwise points to a live mapped `NamedObject`.
     * @param[in] range
     *   Justification: Required by open62541 callback signature for optional numeric subranges.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] data
     *   Justification: Supplies the written `UA_DataValue` payload from the OPC UA client.
     *   Usage: Inspected for `hasValue` and converted via `fromUaVariant(&data->value, ...)`.
     *   Consumed: No (read-only const pointer).
     *   Prerequisites: Non-null `UA_DataValue*` supplied by open62541.
     *
     * @pre data != nullptr
     * @post Target `NamedObject` reflects `&data->value` if `nodeContext != nullptr && data->hasValue`.
     * @throws std::exception [Validation / Contract] Any exception from `fromUaVariant` propagates if uncaught.
     * @error
     *   - Validation / Contract (`nodeContext == nullptr` or `!data->hasValue`): Ignored safely as no-op.
     * @containment
     *   Approach: Null-checks `nodeContext` and `data->hasValue` before converting variant payload.
     *   Limitations: Assumes `data != nullptr` and `nodeContext` lifetime spans `m_objectToNodeMap`.
     */
    static void onWrite(UA_Server *server, const UA_NodeId *sessionId,
                        void *sessionContext, const UA_NodeId *nodeId,
                        void *nodeContext, const UA_NumericRange *range,
                        const UA_DataValue *data);


    /**
     * @brief Static open62541 method-call callback bridging OPC UA calls to `NamedMethod::execute`.
     *
     * @details Validates `methodContext`, deserializes optional JSON string input (`input[0]`)
     * into `NamedObject` arguments, executes the `NamedMethod`, serializes the result to JSON,
     * and writes it to `output[0]` if expected.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0010.14, CS-0010.15, CS-0050.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @compliance Checks pointer nullness explicitly (`methodContext == nullptr`), validates `UA_TYPES_STRING`, and clears temporary `UA_String`.
     *
     * @param[in] server
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Valid `UA_Server*`.
     * @param[in] sessionId
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] sessionContext
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] methodId
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Valid `UA_NodeId*`.
     * @param[in,out] methodContext
     *   Justification: Supplies raw `NamedMethod*` bound during `mapObject()`.
     *   Usage: Checked against `nullptr`, cast to `NamedMethod*`, and invoked via `execute(args)`.
     *   Consumed: Invoked in-place (may mutate target method/tree state).
     *   Prerequisites: Non-null `NamedMethod*` (returns `UA_STATUSCODE_BADINTERNALERROR` if null).
     * @param[in] objectId
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Valid `UA_NodeId*`.
     * @param[in] objectContext
     *   Justification: Required by open62541 `UA_MethodCallback` signature.
     *   Usage: Unused (cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] inputSize
     *   Justification: Reports number of input `UA_Variant` arguments passed by OPC UA client.
     *   Usage: Checked against `0` before indexing `input[0]`.
     *   Consumed: Yes (scalar size value).
     *   Prerequisites: `0` or `>= 1` with matching `input` array.
     * @param[in] input
     *   Justification: Supplies array of `inputSize` OPC UA input variants (expects JSON string at index 0).
     *   Usage: Checked for non-null and `UA_TYPES_STRING` type before deserializing JSON arguments.
     *   Consumed: No (read-only const pointer).
     *   Prerequisites: Non-null when `inputSize > 0`.
     * @param[in] outputSize
     *   Justification: Reports capacity of `output` variant array requested by caller.
     *   Usage: Checked against `0` before writing serialized JSON response to `output`.
     *   Consumed: Yes (scalar size value).
     *   Prerequisites: `0` or `>= 1` matching `output`.
     * @param[out] output
     *   Justification: Receives the JSON-serialized method return value as a `UA_String` variant.
     *   Usage: Populated via `UA_Variant_setScalarCopy` when `outputSize > 0 && output != nullptr`.
     *   Consumed: Mutated in-place (filled with allocated `UA_String` variant).
     *   Prerequisites: Non-null writable `UA_Variant*` array when `outputSize > 0`.
     *
     * @pre (methodContext != nullptr) && (inputSize == 0 || input != nullptr)
     * @post On `UA_STATUSCODE_GOOD`, `output[0]` holds JSON result if `outputSize > 0 && output != nullptr`.
     * @return UA_StatusCode
     *   Justification: Reports method dispatch outcome to open62541 RPC engine.
     *   Usage: Checked by open62541 runtime to compose OPC UA `CallResponse` status.
     *   Consumed: Scalar status code returned by copy.
     *   Prerequisites: Bound `methodContext` and valid input/output descriptors.
     *   Limitations: Expects input argument 0 (if provided) to be scalar `UA_TYPES_STRING`.
     *   Lifecycle: Immediate scalar prvalue; output variant string owned by open62541.
     *   Modification / Deletion: Immutable prvalue; open62541 manages `output` cleanup.
     *   Specific Values: `UA_STATUSCODE_GOOD`, `UA_STATUSCODE_BADINTERNALERROR`, `UA_STATUSCODE_BADTYPEMISMATCH`.
     *   Error Behavior: Returns error status code before execution if `methodContext` is null or input type mismatches.
     * @retval UA_STATUSCODE_GOOD Method executed and response serialized.
     * @retval UA_STATUSCODE_BADINTERNALERROR Null `methodContext` passed to callback.
     * @retval UA_STATUSCODE_BADTYPEMISMATCH Input variant 0 is not `UA_TYPES_STRING`.
     * @throws std::exception [Validation / Contract] Exceptions from `serialization::fromJson`, `execute`, or `toJson` propagate if uncaught.
     * @error
     *   - Validation / Contract (`methodContext == nullptr` or `input[0]` type mismatch): Returns `UA_STATUSCODE_BADINTERNALERROR` or `UA_STATUSCODE_BADTYPEMISMATCH` prior to execution.
     * @containment
     *   Approach: Explicit null and type checks precede JSON parsing and execution; temporary `UA_String` cleared immediately.
     *   Limitations: Does not wrap `NamedMethod::execute` or JSON parsing in a `try`/`catch` inside the C callback.
     */
    static UA_StatusCode onMethodCall(UA_Server *server, const UA_NodeId *sessionId,
                                      void *sessionContext, const UA_NodeId *methodId,
                                      void *methodContext, const UA_NodeId *objectId,
                                      void *objectContext, size_t inputSize,
                                      // Output variant buffer parameters.
                                      const UA_Variant *input, size_t outputSize,
                                      UA_Variant *output);
};



} // namespace quasar::opcua

#endif // QUASAR_OPCUA_OPCUASERVERSERVICE_HPP
