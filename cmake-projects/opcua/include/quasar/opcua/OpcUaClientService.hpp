/**
 * @file OpcUaClientService.hpp
 * @brief Declares OpcUaClientService for mirroring a remote OPC UA address space into a NamedObject tree.
 *
 * @details Defines a `named::NamedService` specialization that connects to an OPC UA server,
 * configures X.509/PKI and username authentication, recursively mirrors remote objects, variables,
 * and methods locally, and serializes cross-thread client calls via a mutex-guarded task queue.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200, FE-0220, FE-0240
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Documents every class field, constructor, destructor, template method, and callback
 * with formal parameter, return, error-containment, and control-structure contracts.
 */

#ifndef QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
#define QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP

// Quasar NamedService base and OPC UA security manager dependencies.
#include "quasar/named/NamedService.hpp"
#include "quasar/opcua/OpcUaSecurityManager.hpp"
// open62541 client APIs and default configuration/subscription headers.
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
// Standard library memory, container, concurrency, and string utilities.
#include <memory>
#include <map>
#include <queue>
#include <functional>
#include <future>
// Mutual exclusion header for thread-safe client state synchronization.
#include <mutex>
// Standard vector and string headers.
#include <vector>
#include <string>

/**
 * @brief Quasar OPC UA integration namespace.
 *
 * @details Hosts OPC UA client, server, security, and variant marshaling abstractions.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Contains `OpcUaClientService` within the `quasar::opcua` namespace.
 */
namespace quasar::opcua {

/**
 * @class OpcUaClientService
 * @brief NamedService that mirrors a remote OPC UA tree locally.
 *
 * @details Handles thread-safety by queuing all client operations to its service thread.
 * Fulfills [TSK-20260311-005.3] OPC UA Client as NamedService and [FE-0200.3] OPC UA Client Service.
 * Connects via open62541, subscribes to remote variable changes, and exposes remote OPC UA
 * methods as `NamedMethod` children backed by `enqueueTask`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200, FE-0220, FE-0240
 * @constraint CS-0010.5, CS-0010.21, CS-0060.5, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `m_queueMutex` serializes every push/swap access to `m_tasks`, and `m_client` is exclusively driven on the service thread after `initializeClient()`.
 * @compliance Isolates non-thread-safe `UA_Client*` calls onto the service cycle via `m_tasks` and cleans up connections in `~OpcUaClientService()`.
 */
class OpcUaClientService : public named::NamedService {
public:
    /**
     * @brief Factory method creating a managed OpcUaClientService instance.
     *
     * @details Allocates the service via a `make_shared_enabler`, binds its reflexive `self`
     * pointer, attaches an optional parent object, and registers the `"run"` service tick method.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `std::make_shared` (no raw `new`) and registers weak-self capture in `"run"` to prevent reference cycles.
     *
     * @param[in] name
     *   Justification: Assigns the unique `NamedObject` identifier for this client service node.
     *   Usage: Forwarded to protected constructor `OpcUaClientService(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty valid object name.
     * @param[in] parent
     *   Justification: Optionally attaches the new service under an existing Quasar parent hierarchy.
     *   Usage: Passed to `self->setParent(parent)` when non-null.
     *   Consumed: No (shared_ptr copied into parent linkage if non-null).
     *   Prerequisites: May be `nullptr`; otherwise a valid `NamedObject` parent.
     *
     * @pre !name.empty()
     * @post Returned `shared_ptr` is non-null, self-bound, and contains a `"run"` `NamedMethod`.
     * @return std::shared_ptr<OpcUaClientService>
     *   Justification: Provides shared ownership required by Quasar's reflexive `NamedObject` tree.
     *   Usage: Caller configures URL/security, starts the service, and queries mirrored children.
     *   Consumed: Ownership shared via returned `std::shared_ptr`; internal `"run"` holds only `weak_ptr`.
     *   Prerequisites: Guaranteed non-null on normal return.
     *   Limitations: Service must be configured (`setUrl`) before calling `start()`.
     *   Lifecycle: Lives until all external `shared_ptr` handles and parent references are released.
     *   Modification / Deletion: Caller may invoke public configuration/lifecycle APIs and drop `shared_ptr` to destroy.
     *   Specific Values: Valid non-null `std::shared_ptr<OpcUaClientService>`.
     *   Error Behavior: Never returns `nullptr`; throws on allocation or parent attachment failure.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared` fails under OOM.
     * @throws std::exception [Validation / Contract] Propagated if `setParent` or `NamedMethod::create` throws.
     * @error
     *   - Resource / Allocation (heap exhaustion): Propagates `std::bad_alloc` with no persistent side effects.
     *   - Validation / Contract (invalid parent or duplicate child name): Propagates `NamedObject` exception; local `self` unwinds cleanly.
     * @containment
     *   Approach: `std::make_shared` and `std::weak_ptr` ensure zero leaks or reference cycles if construction or binding fails.
     *   Limitations: If `setParent` throws after `self` creation, caller never receives the object.
     */
    static std::shared_ptr<OpcUaClientService> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor stopping the client service and disconnecting open62541 resources.
     *
     * @details Invokes `stop()` to halt the service loop, disconnect from the remote server,
     * and free `m_client`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.21, CS-0060.5, CS-0090.1, CS-0090.2
     * @exposed
     * @compliance Guarantees RAII service shutdown and `UA_Client` release on object destruction.
     */
    virtual ~OpcUaClientService();

    /**
     * @brief Sets the remote server endpoint URL.
     *
     * @details Stores the `opc.tcp://` endpoint string used when `start()` invokes
     * `initializeClient()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Simple setter preserving explicit configuration before `start()`.
     *
     * @param[in] url
     *   Justification: Specifies the target OPC UA TCP endpoint to connect to.
     *   Usage: Copied into `m_url` for subsequent `UA_Client_connect`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty valid OPC UA endpoint URL (e.g., `"opc.tcp://localhost:4840"`).
     *
     * @pre !url.empty()
     * @post `m_url == url`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::string` copy allocation fails.
     * @error
     *   - Resource / Allocation (string assignment OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Direct member string assignment prior to connection setup.
     *   Limitations: Must not be called concurrently with `initializeClient()`; does not validate URL syntax eagerly.
     */
    void setUrl(const std::string& url);

    /**
     * @brief Starts the client and connection.
     *
     * @details Guards against duplicate starts via `isRunning()`, connects and mirrors the remote
     * tree via `initializeClient()`, configures the 10 ms service cycle, and starts `NamedService`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.20, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards idempotent start and delegates worker thread startup to `NamedService::start()`.
     *
     * @pre Service is stopped (`!isRunning()`) and `m_url` points to a reachable OPC UA endpoint.
     * @post On normal return, client is connected, tree is mirrored, and service thread is running.
     * @throws std::runtime_error [Hardware / Bus / I/O] Propagated from `initializeClient()` if connection fails.
     * @error
     *   - Validation / Contract (already running): Returns early as an idempotent no-op.
     *   - Hardware / Bus / I/O (server unreachable or handshake failure): `initializeClient()` cleans up `m_client` and throws `std::runtime_error`.
     * @containment
     *   Approach: Idempotency guard (`isRunning()`) prevents double connection, and `initializeClient()` deletes `m_client` if connect fails before thread launch.
     *   Limitations: Does not automatically reconnect if network drops after `start()` succeeds.
     */
    void start() override;

    /**
     * @brief Stops the client and connection.
     *
     * @details Checks `isRunning()`, stops the `NamedService` background thread, disconnects
     * `m_client`, deletes the open62541 client handle, and resets `m_client` to `nullptr`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Stops worker thread before disconnecting/freeing `m_client` to prevent use-after-free.
     *
     * @post `!isRunning()` and `m_client == nullptr`.
     * @throws std::exception [Resource / Fatal] Propagated if `NamedService::stop()` throws during thread join.
     * @error
     *   - Validation / Contract (already stopped): Returns early as a no-op.
     *   - Resource / Fatal (`NamedService::stop()` failure): Propagates exception before client teardown.
     * @containment
     *   Approach: Halts background execution via `NamedService::stop()` prior to tearing down `m_client` and nulling the pointer.
     *   Limitations: Any queued tasks remaining in `m_tasks` after `stop()` are not executed.
     */
    void stop() override;

    /**
     * @brief Gets the type name.
     *
     * @details Overrides `NamedObject::getType()` to return the literal class identifier.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Provides deterministic reflexive type identification.
     *
     * @post Returned string equals `"OpcUaClientService"`.
     * @return std::string
     *   Justification: Enables Quasar reflexive type dispatch and serialization lookup.
     *   Usage: Caller compares returned string against `"OpcUaClientService"`.
     *   Consumed: Value returned by move/NRVO; caller owns string copy.
     *   Prerequisites: None.
     *   Limitations: Fixed constant string return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may modify or destroy returned string freely.
     *   Specific Values: Always `"OpcUaClientService"`.
     *   Error Behavior: May throw `std::bad_alloc` only if small-string optimization is bypassed under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible if string construction allocates.
     * @error
     *   - Resource / Allocation (string allocation failure): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Returns short literal string directly (eligible for SSO on standard libraries).
     *   Limitations: Signature cannot be `noexcept` due to base class override.
     */
    std::string getType() const override;

    /**
     * @brief Loads client certificate and private key.
     *
     * @details Delegates DER certificate and private key loading to `m_securityManager.loadCertificate`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates PKI key management to `OpcUaSecurityManager`.
     *
     * @param[in] certPath
     *   Justification: Identifies the client DER X.509 certificate file path.
     *   Usage: Forwarded to `m_securityManager.loadCertificate`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty path to readable DER certificate file.
     * @param[in] keyPath
     *   Justification: Identifies the client DER private key file path.
     *   Usage: Forwarded to `m_securityManager.loadCertificate`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty path to readable DER private key file.
     *
     * @pre !certPath.empty() && !keyPath.empty()
     * @post Security manager holds updated client identity if files loaded cleanly.
     * @throws std::bad_alloc [Resource / Allocation] Propagated if underlying file load allocations fail.
     * @error
     *   - Hardware / Bus / I/O (missing or unreadable file): `m_securityManager.loadCertificate` ignores failure and keeps prior identity; note return status is not surfaced here.
     * @containment
     *   Approach: Delegates to `OpcUaSecurityManager::loadCertificate`, which validates both files before swapping identity.
     *   Limitations: Void signature does not return `UA_StatusCode` failure to caller.
     */
    void loadCertificate(const std::string& certPath, const std::string& keyPath);

    /**
     * @brief Loads trust list for server certificate validation.
     *
     * @details Delegates loading of trusted DER certificates to `m_securityManager.loadTrustList`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates trusted certificate loading to `OpcUaSecurityManager`.
     *
     * @param[in] trustListPaths
     *   Justification: Supplies paths to trusted server/CA certificates in DER format.
     *   Usage: Forwarded to `m_securityManager.loadTrustList`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Vector of valid DER certificate paths (may be empty).
     *
     * @pre true (empty vector is valid).
     * @post Valid trust certificates are appended to `m_securityManager`.
     * @throws std::bad_alloc [Resource / Allocation] Propagated if vector growth fails.
     * @error
     *   - Hardware / Bus / I/O (individual unreadable trust file): Skipped cleanly inside `m_securityManager`.
     * @containment
     *   Approach: Delegates to `OpcUaSecurityManager::loadTrustList`.
     *   Limitations: Void signature does not report skipped/unreadable files.
     */
    void loadTrustList(const std::vector<std::string>& trustListPaths);

    /**
     * @brief Sets the credentials for username/password authentication.
     *
     * @details Stores `username` and `password` internally for `initializeClient()` to pass to
     * `UA_ClientConfig_setAuthenticationUsername`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Captures authentication credentials prior to `start()`.
     *
     * @param[in] username
     *   Justification: Supplies the OPC UA username identity for session authentication.
     *   Usage: Stored in `m_username` and checked for non-emptiness in `initializeClient()`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Valid username string (empty string disables username auth).
     * @param[in] password
     *   Justification: Supplies the password paired with `username`.
     *   Usage: Stored in `m_password` and passed to open62541 authentication setup.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Valid password string matching server credentials.
     *
     * @pre true (empty username clears username authentication).
     * @post `m_username == username && m_password == password`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if string copy fails.
     * @error
     *   - Resource / Allocation (string assignment OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Direct member assignment before connection startup.
     *   Limitations: Credentials remain in plaintext `std::string` in memory while service exists; must be set before `start()`.
     */
    void setCredentials(const std::string& username, const std::string& password);

    /**
     * @brief Queues a task to be executed on the client thread.
     *
     * @details Wraps `task` with a `std::promise<T>`, enqueues the lambda onto `m_tasks` under
     * `m_queueMutex`, and returns the associated `std::future<T>` so callers can wait for the
     * result generated safely on the service thread.
     *
     * @tparam T Return type produced by the queued `UA_Client*` task callback.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.5, CS-0010.20, CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `std::lock_guard<std::mutex>` RAII locking and `try`/`catch` exception forwarding via `std::promise`.
     *
     * @param[in] task
     *   Justification: Encapsulates a `UA_Client*` operation that must execute on the client thread.
     *   Usage: Captured by value into the queued wrapper lambda and invoked by `processTasks()`.
     *   Consumed: Copied into queue closure (caller's `std::function` remains intact).
     *   Prerequisites: Valid callable `task` (`static_cast<bool>(task) == true`) safe to execute with `m_client`.
     *
     * @pre static_cast<bool>(task) == true
     * @post Task wrapper is pushed into `m_tasks` under `m_queueMutex` and linked to returned `std::future<T>`.
     * @return std::future<T>
     *   Justification: Allows off-thread callers to await asynchronous client-thread execution and exceptions.
     *   Usage: Caller invokes `wait_for(...)` and `get()` to retrieve result `T` or rethrow exception.
     *   Consumed: Ownership transferred to caller via move/NRVO.
     *   Prerequisites: Service must be running so `processTasks()` drains the queue.
     *   Limitations: Blocks indefinitely if `get()` is called without timeout after service stops.
     *   Lifecycle: Valid until `get()` consumes the shared state or future is destroyed.
     *   Modification / Deletion: Caller may move, wait on, or discard returned future; no manual free needed.
     *   Specific Values: Valid `std::future<T>` linked to `promise`.
     *   Error Behavior: Exceptions thrown inside `task` are captured into `promise->set_exception` and rethrown on `future.get()`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared<std::promise<T>>` or `m_tasks.push` fails.
     * @error
     *   - Resource / Allocation (queue push or promise allocation OOM): Propagates before queue insertion; lock released via RAII.
     *   - Validation / Contract (exception thrown inside `task`): Captured inside `try`/`catch (...)` on service thread and marshaled into `std::future<T>`.
     * @containment
     *   Approach: `std::lock_guard` limits lock hold to queue push, and `try`/`catch (...)` prevents
     *   worker-thread task exceptions from terminating the service thread.
     *   Limitations: Uses unbounded `std::mutex` (queue push is O(1)); queued tasks may never resolve if `stop()` occurs before drain.
     */
    template<typename T>
    std::future<T> enqueueTask(std::function<T(UA_Client*)> task) {
        // Allocate shared promise so queued closure and caller future share lifetime safely.
        auto promise = std::make_shared<std::promise<T>>();
        {
            // Serialize queue insertion against service-thread queue swap [CS-0010.21].
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_tasks.push([task, promise](UA_Client* client) {
                // Capture task exceptions into promise instead of unwinding service thread [CS-0010.20].
                try {
                    promise->set_value(task(client));
                } catch (...) {
                    // Forward current exception to caller waiting on std::future<T>.
                    promise->set_exception(std::current_exception());
                }
            });
        }
        // Return future associated with the queued promise.
        return promise->get_future();
    }

    /**
     * @brief Specialization for void tasks queued on the client thread.
     *
     * @details Wraps a void-returning `UA_Client*` task with a `std::promise<void>`,
     * pushes it onto `m_tasks` under `m_queueMutex`, and returns `std::future<void>`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.5, CS-0010.20, CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `std::lock_guard<std::mutex>` RAII locking and `try`/`catch` exception marshaling for void tasks.
     *
     * @param[in] task
     *   Justification: Encapsulates a void `UA_Client*` operation to run on the client service thread.
     *   Usage: Copied into the queued closure and invoked inside `processTasks()`.
     *   Consumed: Copied into queue closure (caller's `std::function` remains intact).
     *   Prerequisites: Valid callable `task` (`static_cast<bool>(task) == true`).
     *
     * @pre static_cast<bool>(task) == true
     * @post Task closure is queued in `m_tasks` and linked to returned `std::future<void>`.
     * @return std::future<void>
     *   Justification: Synchronizes completion and exception propagation of void client tasks.
     *   Usage: Caller awaits `wait_for(...)` and invokes `get()` to observe completion or errors.
     *   Consumed: Ownership transferred to caller via move/NRVO.
     *   Prerequisites: Service must be running so `processTasks()` executes the task.
     *   Limitations: Waiting without timeout hangs if service stops before queue drain.
     *   Lifecycle: Valid until `get()` or future destruction.
     *   Modification / Deletion: Caller may move, wait on, or discard future; no manual deletion.
     *   Specific Values: Valid `std::future<void>` linked to internal promise.
     *   Error Behavior: Exceptions inside `task` are captured into `promise` and rethrown on `future.get()`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if promise allocation or queue push fails.
     * @error
     *   - Resource / Allocation (promise/queue OOM): Propagates cleanly; mutex unlocked via RAII.
     *   - Validation / Contract (exception thrown by `task`): Stored in `promise` via `try`/`catch (...)`.
     * @containment
     *   Approach: RAII `std::lock_guard` protects `m_tasks`, and `try`/`catch (...)` isolates task exceptions from the service thread.
     *   Limitations: Queued tasks not drained prior to `stop()` leave futures unfulfilled.
     */
    std::future<void> enqueueTask(std::function<void(UA_Client*)> task) {
        // Allocate shared void promise for completion/exception signaling.
        auto promise = std::make_shared<std::promise<void>>();
        {
            // Protect task queue mutation with RAII mutex guard [CS-0010.21].
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_tasks.push([task, promise](UA_Client* client) {
                // Execute void task inside exception guard to keep service loop alive [CS-0010.20].
                try {
                    task(client);
                    promise->set_value();
                } catch (...) {
                    // Propagate task exception to caller's std::future<void>.
                    promise->set_exception(std::current_exception());
                }
            });
        }
        // Return completion future to caller.
        return promise->get_future();
    }

protected:
    /**
     * @brief Constructor initializing base NamedService with service name.
     *
     * @details Protected constructor enforced so callers use `OpcUaClientService::create()`
     * ensuring proper `std::shared_ptr` and reflexive `setSelf` initialization.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Enforces `explicit` single-argument construction via factory pattern.
     *
     * @param[in] name
     *   Justification: Supplies the Quasar node name passed to `NamedService`.
     *   Usage: Forwarded to `NamedService(name)` base initializer.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name string.
     *
     * @pre !name.empty()
     * @post Base `NamedService` initialized with `name` and default `m_url`.
     * @throws std::exception [Resource / Allocation] Propagates any exception thrown by `NamedService`.
     * @error
     *   - Resource / Allocation (base construction failure): Propagates cleanly.
     * @containment
     *   Approach: Protected access restricts instantiation to `create()`.
     *   Limitations: Reflexive `self` pointer must be bound immediately afterward in `create()`.
     */
    explicit OpcUaClientService(const std::string& name);

    /**
     * @brief Initializes the client and browses the remote tree.
     *
     * @details Allocates `UA_Client`, configures default client settings, applies
     * `m_securityManager`, configures optional username/password login, connects to `m_url`,
     * creates a subscription, and calls `browseAndMirror` from `ObjectsFolder`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200, FE-0220
     * @constraint CS-0010.20, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @compliance Cleans up `m_client` via `UA_Client_delete` and resets to `nullptr` before throwing on connect error.
     *
     * @pre `m_client == nullptr` and `m_url` is set.
     * @post On normal return, `m_client != nullptr`, connected, and remote address space is mirrored.
     * @throws std::runtime_error [Hardware / Bus / I/O] Thrown if `UA_Client_connect` does not return `UA_STATUSCODE_GOOD`.
     * @error
     *   - Hardware / Bus / I/O (connect failure): Deletes `m_client`, sets it to `nullptr`, and throws `std::runtime_error`.
     * @containment
     *   Approach: Immediately deletes and nulls `m_client` on connect failure before throwing.
     *   Limitations: Subscription creation failure is tolerated without throwing (disables monitored items).
     */
    void initializeClient();

    /**
     * @brief Recursively browses the remote server and creates local objects.
     *
     * @details Issues `UA_Client_Service_browse` on `remoteNodeId`, filters standard namespace-0
     * system containers (`Server`, `Types`, `Views`), creates `NamedMethod` proxies or typed
     * `NamedObject` mirrors, attaches data-change monitored items for variables, and recurses.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Clears `UA_BrowseRequest`, `UA_BrowseResponse`, and temporary `UA_NodeId`/`UA_Variant` buffers on every path.
     *
     * @param[in] remoteNodeId
     *   Justification: Identifies the remote OPC UA node whose forward references are browsed.
     *   Usage: Copied into `UA_BrowseRequest` and used as `objectId` for child method bindings.
     *   Consumed: No (passed by value `UA_NodeId` struct).
     *   Prerequisites: Valid `UA_NodeId` on the connected server.
     * @param[in,out] localParent
     *   Justification: Specifies the local Quasar container node under which mirrored children attach.
     *   Usage: Passed as parent to `NamedMethod::create` or `localObj->setParent(localParent)`.
     *   Consumed: Mutated in-place (receives mirrored child `NamedObject` entries).
     *   Prerequisites: Non-null `std::shared_ptr<named::NamedObject>`.
     *
     * @pre `m_client != nullptr && localParent != nullptr`
     * @post Remote children of `remoteNodeId` (excluding system nodes) are mirrored under `localParent`.
     * @throws std::exception [Resource / Allocation] Propagates if local `NamedObject` creation throws.
     * @error
     *   - Hardware / Bus / I/O (browse or read returns empty/error): Empty result set skips children without crashing; `UA_BrowseRequest`/`Response` cleared.
     * @containment
     *   Approach: Frees open62541 `UA_BrowseRequest`, `UA_BrowseResponse`, `UA_NodeId`, and `UA_Variant` resources at scope end; skips namespace-0 system subtrees (`id < 100`).
     *   Limitations: Recursive traversal depth follows remote server tree depth; cyclic non-ns0 references are not explicitly guarded.
     */
    void browseAndMirror(UA_NodeId remoteNodeId, std::shared_ptr<named::NamedObject> localParent);

    /**
     * @brief Processes all pending tasks in the queue.
     *
     * @details Atomically swaps `m_tasks` into a local queue under `m_queueMutex` and invokes
     * each queued callable with `m_client` outside the lock.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @compliance Swaps queue under `std::lock_guard` so `m_queueMutex` is not held during `UA_Client*` execution.
     *
     * @pre Invoked exclusively on the client service thread while `m_client` is valid.
     * @post `m_tasks` queue snapshot is drained.
     * @throws std::exception [Validation / Contract] Any uncaught exception from a task wrapper propagates (wrappers created by `enqueueTask` catch internally).
     * @error
     *   - Validation / Contract (task internal exception): Contained inside `enqueueTask` wrapper via `std::promise::set_exception`.
     * @containment
     *   Approach: Queue swap minimizes mutex hold time and avoids deadlock if a task enqueues another task.
     *   Limitations: Unbounded queue drain processes all currently queued items in one cycle.
     */
    void processTasks();

private:
    /**
     * @brief Open62541 client connection handle driven exclusively on the service thread.
     * @details Allocated in `initializeClient()` and disconnected/freed in `stop()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized to `nullptr` and cleaned up in `stop()` / `~OpcUaClientService()`.
     */
    UA_Client* m_client{nullptr};

    /**
     * @brief Remote OPC UA server endpoint URL configured prior to `start()`.
     * @details Defaults to `"opc.tcp://localhost:4840"` and updated via `setUrl()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized with a default endpoint string.
     */
    std::string m_url{"opc.tcp://localhost:4840"};
    
    /**
     * @brief Security manager owning client X.509 certificate, private key, and trust list.
     * @details Applied during `initializeClient()` via `configureClient()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2
     * @compliance RAII member automatically releases PKI buffers on service destruction.
     */
    OpcUaSecurityManager m_securityManager;

    /**
     * @brief Username credential used for OPC UA `UserNameIdentityToken` authentication.
     * @details Set via `setCredentials()`; empty disables username authentication.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Default-initialized empty string.
     */
    std::string m_username;

    /**
     * @brief Password credential paired with `m_username`.
     * @details Set via `setCredentials()` and supplied to `UA_ClientConfig_setAuthenticationUsername`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200, FE-0220
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Default-initialized empty string.
     */
    std::string m_password;

    /**
     * @brief Lookup map associating serialized remote NodeId keys with local `NamedObject` mirrors.
     * @details Reserved for tracking remote-to-local node associations across the mirrored subtree.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Uses `std::shared_ptr` values managed by standard `std::map`.
     */
    std::map<std::string, std::shared_ptr<named::NamedObject>> m_nodeToLocalMap;

    /**
     * @brief Mutex synchronizing cross-thread insertions and swaps on `m_tasks`.
     * @details Acquired via `std::lock_guard` in `enqueueTask()` and `processTasks()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.5, CS-0010.21, CS-0090.1, CS-0090.2
     * @compliance Guards all shared access to `m_tasks` using RAII lock guards.
     */
    std::mutex m_queueMutex;

    /**
     * @brief FIFO queue of pending `UA_Client*` closures submitted by external threads.
     * @details Guarded by `m_queueMutex` and drained on the service thread in `processTasks()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.5, CS-0090.1, CS-0090.2
     * @compliance Exclusively accessed while holding `m_queueMutex`.
     */
    std::queue<std::function<void(UA_Client*)>> m_tasks;

    /**
     * @brief The single subscription ID for monitoring all variables.
     * @details Assigned in `initializeClient()` when `UA_Client_Subscriptions_create` succeeds; `0` if inactive.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.32, CS-0090.1, CS-0090.2
     * @compliance Explicitly initialized to `0`.
     */
    UA_UInt32 m_subscriptionId{0};

    /**
     * @brief Callback for data change notifications from open62541 monitored items.
     *
     * @details Casts `monContext` to `named::NamedObject*` and updates the mirrored local node
     * via `fromUaVariant(&value->value, obj->getSelf())` when `value->hasValue` is true.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @compliance Guards `monContext` and `value->hasValue` prior to dereferencing and type conversion.
     *
     * @param[in] client
     *   Justification: Required by open62541 `UA_Client_DataChangeNotificationCallback` signature.
     *   Usage: Unused (explicitly cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: Passed by open62541 runtime during subscription iterate.
     * @param[in] subId
     *   Justification: Required by open62541 callback signature to identify subscription.
     *   Usage: Unused (explicitly cast to `void`).
     *   Consumed: No (scalar value).
     *   Prerequisites: Matches `m_subscriptionId`.
     * @param[in] subContext
     *   Justification: Required by open62541 callback signature for subscription user context.
     *   Usage: Unused (explicitly cast to `void`).
     *   Consumed: No (unused pointer).
     *   Prerequisites: May be `nullptr`.
     * @param[in] monId
     *   Justification: Required by open62541 callback signature to identify monitored item.
     *   Usage: Unused (explicitly cast to `void`).
     *   Consumed: No (scalar value).
     *   Prerequisites: Valid monitored item identifier.
     * @param[in,out] monContext
     *   Justification: Supplies the raw pointer to the mirrored `named::NamedObject` instance.
     *   Usage: Cast to `NamedObject*` to resolve `getSelf()` and update value via `fromUaVariant`.
     *   Consumed: Mutated indirectly (pointed-to `NamedObject` value updated when `value->hasValue`).
     *   Prerequisites: May be `nullptr` (guarded); if non-null, must point to a live `NamedObject` owned by the mirrored tree.
     * @param[in] value
     *   Justification: Delivers the updated `UA_DataValue` received from the OPC UA server.
     *   Usage: Checked for `hasValue` and passed to `fromUaVariant`.
     *   Consumed: No (read-only pointer owned by open62541).
     *   Prerequisites: Non-null `UA_DataValue*` supplied by open62541.
     *
     * @pre value != nullptr
     * @post Target mirrored `NamedObject` reflects `&value->value` if `monContext != nullptr && value->hasValue`.
     * @throws std::exception [Validation / Contract] Any exception from `fromUaVariant` or `NamedObject::setValue` could propagate into C callback if uncaught.
     * @error
     *   - Validation / Contract (`monContext == nullptr` or `!value->hasValue`): Ignored safely as no-op.
     * @containment
     *   Approach: Null-checks `monContext` and `value->hasValue` before casting or modifying local nodes.
     *   Limitations: Assumes `value != nullptr` and `monContext` outlives the monitored item subscription.
     */
    static void onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                             UA_UInt32 monId, void *monContext, UA_DataValue *value);
};

} // namespace quasar::opcua


#endif // QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
