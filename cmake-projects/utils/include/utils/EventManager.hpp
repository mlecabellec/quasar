/**
 * @file EventManager.hpp
 * @brief Implementation of the SMP Event Manager Service.
 *
 * @details This file declares the `utils::EventManager` service class providing
 * mechanisms for SMP event identification, subscription, unsubscription, and
 * synchronous or asynchronous-flagged emission under timed-mutex protection.
 * Contributes to [FE-0070.4.1].
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0090.1, CS-0090.2
 * @compliance Declares `utils::EventManager` in a dedicated self-contained header
 * inside namespace `utils` with formal Doxygen traceability and contracts.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP IEventManager service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`EventManager.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_nextEventId`, `_eventIds`, `_subscriptions` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `QueryEventId`, `Subscribe`, `Emit` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. It implements event management services and does not provide data types such as `BitBufferSlice`, `NamedObject` hierarchies, or codec implementations required for binary serialization and deserialization.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP IEventManager service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.4 (Event Manager):
 * - Implements the IEventManager component [FE-0070.4.1].
 * - Supports event identification via `QueryEventId` [FE-0070.4.2].
 * - Maintains internal lists for event IDs and entry points, fulfilling [FE-0070.4.3] and [FE-0070.4.4] (initially empty).
 * - Provides `Subscribe` [FE-0070.4.5] and `Unsubscribe` [FE-0070.4.6] functionality.
 * - Implements `Emit` for global event emission [FE-0070.4.7], supporting synchronous flag [FE-0070.4.9].
 * - Registers predefined global events, aligning with [FE-0070.4.8].
 * - @warning The implementation of [FE-0070.4.10] (State transition event shall not trigger another transition) is not explicitly handled in the Emit logic.
 * - The copying of subscribers during Emit helps address [FE-0070.4.11].
 */
#pragma once

// SMP and Quasar core base interfaces.
#include <Smp/IObject.h>
#include <Smp/Services/IEventManager.h>
#include <core/Object.hpp>

// Standard library containers and timed synchronization primitives.
#include <map>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief Namespace containing core SMP utility service implementations.
 * @details Groups concrete SMP service adapters (`EventManager`, `Logger`,
 * `TimeKeeper`) within the `utils` module namespace.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Encloses all utility service declarations within a dedicated namespace.
 */
namespace utils {

/**
 * @brief Implementation of the SMP Event Manager Service.
 * @details This class provides mechanisms for event subscription, unsubscription, and emission.
 * Contributes to [FE-0070.3.1] and [FE-0070.4.1]. All shared maps and subscription vectors are
 * guarded by `_mutex` (`std::timed_mutex`) with bounded 1-second acquisition timeouts, and `Emit`
 * snapshots subscriber lists before invoking callbacks outside the lock to prevent re-entrant deadlocks.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed SMP `IEventManager` and `IComponent` service boundary.
 * @compliance Uses `std::timed_mutex` for bounded locking, `const` correctness on accessors,
 * RAII lock guards, and explicit container types without `auto`.
 * @invariant `_mutex` serializes all concurrent reads and writes to `_nextEventId`, `_eventIds`,
 * `_eventNames`, and `_subscriptions`; `_nextEventId >= 20` preserves SMP predefined IDs.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 */
class EventManager : public core::Object,
                     public virtual Smp::Services::IEventManager {
public:
  /**
   * @brief Default constructor.
   * @details Initializes the underlying `core::Object` with service metadata and
   * populates `_eventIds` with all standard SMP predefined global events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.45, CS-0090.1, CS-0090.2
   * @compliance Deterministic construction with predefined SMP event registration.
   * @post Predefined SMP events are registered in `_eventIds` and `_subscriptions` is empty.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if internal map insertion fails during predefined event registration.
   * @error
   *   - Resource / Allocation (`std::bad_alloc`): Standard container allocation failure aborts construction with no leaked state.
   * @containment
   *   Approach: Member containers clean up automatically via RAII if construction throws.
   *   Limitations: Allocation failure during predefined event registration cannot be recovered internally and propagates to the caller.
   */
  EventManager();

  /**
   * @brief Virtual destructor.
   * @details Releases owned maps, vectors, and synchronization primitives via RAII member destruction.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0060.5, CS-0090.1, CS-0090.2
   * @compliance Marked `noexcept` and `override` with RAII-managed members only.
   */
  virtual ~EventManager() noexcept override = default;

  // IComponent methods

  /**
   * @brief Query the current SMP component lifecycle state.
   * @details Returns the fixed connected operational state for this utility service.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetState` interface.
   * @compliance Const-qualified accessor returning explicit SMP enumeration state without mutation.
   * @post Returns `Smp::ComponentStateKind::CSK_Connected` without altering `*this`.
   * @return Smp::ComponentStateKind
   *   Justification: Exposes the component lifecycle state required by `Smp::IComponent`.
   *   Usage: Caller inspects the returned enum to confirm the service is connected.
   *   Consumed: Value returned by copy; no internal state is consumed or transferred.
   *   Prerequisites: Valid `EventManager` instance; no lock or prior configuration required.
   *   Limitations: Reports a static `CSK_Connected` state regardless of `Connect`/`Disconnect` calls.
   *   Lifecycle: Standalone temporary prvalue valid in caller expression scope.
   *   Modification / Deletion: Caller may copy or bind to mutable local; no cleanup required.
   *   Specific Values: Always returns `Smp::ComponentStateKind::CSK_Connected`.
   *   Error Behavior: Never fails or throws in current implementation.
   * @error
   *   - None (no-throw return path): Returns `CSK_Connected` unconditionally.
   * @containment
   *   Approach: Pure constant-returning accessor performs no allocations or I/O.
   *   Limitations: Signature omits `noexcept` per `Smp::IComponent` ABI; state tracking is stubbed to `CSK_Connected`.
   */
  Smp::ComponentStateKind GetState() const override;

  /**
   * @brief Publish component properties to an SMP publication receiver.
   * @details Validates the receiver pointer against null and returns immediately because
   * `EventManager` exposes no published SMP fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Publish` interface.
   * @compliance Guards against null receiver pointer per [CS-0010.15] before no-op return.
   * @param[in] receiver
   *   Justification: Required by the `Smp::IComponent::Publish` interface to register published items.
   *   Usage: Checked for `nullptr` and otherwise ignored since no properties are published.
   *   Consumed: No (non-owning pointer; neither dereferenced nor stored).
   *   Prerequisites: May be `nullptr` (safely ignored) or a valid `Smp::IPublication` pointer.
   * @pre `receiver == nullptr || receiver != nullptr` (null pointer is handled safely).
   * @post `*this` remains unchanged.
   * @error
   *   - Validation / Contract (`receiver == nullptr`): Safely ignored with early return; no-throw.
   * @containment
   *   Approach: Explicit null check prevents invalid dereference even if extended later.
   *   Limitations: Does not publish diagnostic counters or internal event tables to `receiver`.
   */
  void Publish(Smp::IPublication *receiver) override;

  /**
   * @brief Configure the component with logger and link registry services.
   * @details Stub implementation of `Smp::IComponent::Configure`; parameters are retained
   * for interface compatibility and no internal state is mutated.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Configure` interface.
   * @compliance Satisfies `Smp::IComponent` configuration contract without side effects.
   * @param[in] logger
   *   Justification: Required by `Smp::IComponent::Configure` to inject a logger service.
   *   Usage: Unused in the current stub implementation.
   *   Consumed: No (non-owning pointer; not dereferenced or retained).
   *   Prerequisites: May be `nullptr` or a valid `Smp::Services::ILogger` pointer.
   * @param[in] linkRegistry
   *   Justification: Required by `Smp::IComponent::Configure` to inject a link registry.
   *   Usage: Unused in the current stub implementation.
   *   Consumed: No (non-owning pointer; not dereferenced or retained).
   *   Prerequisites: May be `nullptr` or a valid `Smp::Services::ILinkRegistry` pointer.
   * @pre `true` (null pointers are tolerated by this no-op override).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Accepts any pointer values without failure.
   * @containment
   *   Approach: No-op body isolates configuration lifecycle calls from runtime event state.
   *   Limitations: Does not validate service pointers or attach external logger/link dependencies.
   */
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;

  /**
   * @brief Connect the component to the hosting SMP simulator.
   * @details Stub implementation of `Smp::IComponent::Connect`; the simulator pointer
   * is not stored or dereferenced.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Connect` interface.
   * @compliance Implements the required `Smp::IComponent` connection hook safely as a no-op.
   * @param[in] simulator
   *   Justification: Required by `Smp::IComponent::Connect` to bind the component to a simulator.
   *   Usage: Unused in the current stub implementation.
   *   Consumed: No (non-owning pointer; not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or a valid `Smp::ISimulator` pointer.
   * @pre `true` (null pointer is tolerated by this no-op override).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Returns cleanly without modifying state.
   * @containment
   *   Approach: No-op implementation prevents null-dereference or premature state coupling.
   *   Limitations: Simulator pointer is not retained for downstream service discovery.
   */
  void Connect(Smp::ISimulator *simulator) override;

  /**
   * @brief Disconnect the component from the hosting SMP simulator.
   * @details Stub implementation of `Smp::IComponent::Disconnect`; preserves existing event
   * registrations and subscriptions unchanged.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Disconnect` interface.
   * @compliance Preserves object integrity across disconnect lifecycle notifications.
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Disconnects cleanly with no failure paths.
   * @containment
   *   Approach: No-op body avoids tearing down active subscriptions unexpectedly.
   *   Limitations: Does not clear `_subscriptions` or transition state away from `CSK_Connected`.
   */
  void Disconnect() override;

  /**
   * @brief Retrieve the unique SMP type UUID of this component.
   * @details Returns a reference to a function-local static `Smp::Uuid` identifying the
   * EventManager service, avoiding global constants per [CS-0010.30].
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.30, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetUuid` interface.
   * @compliance Uses a function-local static UUID instead of a global constant and preserves constness.
   * @post Returned reference addresses a valid static `Smp::Uuid` `{0, 0, 0, 0, 4}`.
   * @return const Smp::Uuid &
   *   Justification: Satisfies `Smp::IComponent::GetUuid` without copying the 16-byte UUID.
   *   Usage: Caller reads or copies the UUID value for service identification.
   *   Consumed: Borrowed const reference; ownership remains with the static storage duration object.
   *   Prerequisites: None; safe to invoke in any component state.
   *   Limitations: Returns a generic service placeholder UUID `{0, 0, 0, 0, 4}` shared with static storage.
   *   Lifecycle: Static storage duration; valid for the lifetime of the process.
   *   Modification / Deletion: Immutable (`const &`); caller MUST NOT cast away const or delete.
   *   Specific Values: Reference to `{0, 0, 0, 0, 4}`.
   *   Error Behavior: Never fails or throws after initialization.
   * @error
   *   - None (no-throw constant reference): Returns static UUID reference safely.
   * @containment
   *   Approach: Function-local static initialization provides thread-safe C++ initialization without heap usage.
   *   Limitations: Signature omits `noexcept` due to base `Smp::IComponent::GetUuid` virtual declaration.
   */
  const Smp::Uuid &GetUuid() const override;

  /**
   * @brief Retrieve a published field by full name.
   * @details Stub override returning `nullptr` because `EventManager` does not expose SMP fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetField` interface.
   * @compliance Const-qualified override returning a safe null sentinel for unexposed fields.
   * @param[in] fullName
   *   Justification: Required by `Smp::IComponent::GetField` to identify the requested field path.
   *   Usage: Ignored in this stub implementation since no fields exist.
   *   Consumed: No (read-only C-string pointer; neither dereferenced nor stored).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely by stub).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IField *
   *   Justification: Communicates absence of the named field per `Smp::IComponent::GetField`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr` return.
   *   Limitations: Always returns `nullptr` because no SMP fields are published.
   *   Lifecycle: Not applicable (returns `nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete the returned `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Returns `nullptr` for any input without throwing.
   * @retval nullptr No fields are exposed by `EventManager`.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Avoids dereferencing `fullName` and returns `nullptr` directly.
   *   Limitations: Callers attempting field reflection receive `nullptr` and must handle absence.
   */
  Smp::IField *GetField(Smp::String8 fullName) const override;

  /**
   * @brief Retrieve the collection of all published fields.
   * @details Stub override returning `nullptr` because `EventManager` publishes no SMP fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetFields` interface.
   * @compliance Const-qualified override returning `nullptr` when no field collection exists.
   * @post Returns `nullptr` without modifying `*this`.
   * @return const Smp::FieldCollection *
   *   Justification: Conforms to `Smp::IComponent::GetFields` for components without fields.
   *   Usage: Caller checks pointer against `nullptr` before iterating.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must test for `nullptr`.
   *   Limitations: Always returns `nullptr`; field introspection is unsupported.
   *   Lifecycle: Not applicable (returns `nullptr`).
   *   Modification / Deletion: Immutable null pointer; caller MUST NOT dereference or delete.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never fails or throws.
   * @retval nullptr Component exposes no field collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless return of `nullptr`.
   *   Limitations: Field enumeration callers must tolerate `nullptr` collections.
   */
  const Smp::FieldCollection *GetFields() const override;

  /**
   * @brief Read the simple value of a named field.
   * @details Stub override returning a default-constructed `Smp::AnySimple` because
   * `EventManager` exposes no simple fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleValue` interface.
   * @compliance Const-qualified override returning an initialized `Smp::AnySimple` by value.
   * @param[in] fullName
   *   Justification: Required by `Smp::IComponent::GetSimpleValue` to identify the target field.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: No (read-only C-string pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely by stub).
   * @post Returns default `Smp::AnySimple()` without altering `*this`.
   * @return Smp::AnySimple
   *   Justification: Satisfies the `Smp::IComponent::GetSimpleValue` return type.
   *   Usage: Caller inspects `.type` to observe an unpopulated `ST_None` value.
   *   Consumed: Value returned by move/copy; caller owns the returned temporary.
   *   Prerequisites: None; verify `.type` before extracting typed payload.
   *   Limitations: Always returns an empty `Smp::AnySimple` because no fields exist.
   *   Lifecycle: Standalone caller-owned value object.
   *   Modification / Deletion: Caller may modify or discard its returned copy; no manual delete.
   *   Specific Values: Default-constructed `Smp::AnySimple` (`ST_None`).
   *   Error Behavior: Returns empty `Smp::AnySimple` without throwing.
   * @error
   *   - None (no-throw stub): Always returns default `Smp::AnySimple`.
   * @containment
   *   Approach: Returns value-initialized sentinel object without touching `fullName`.
   *   Limitations: Does not throw `InvalidFieldName` for missing fields.
   */
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;

  /**
   * @brief Set the simple value of a named field.
   * @details Stub override; performs no action because `EventManager` exposes no simple fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleValue` interface.
   * @compliance Safely implements the `Smp::IComponent` mutator interface as a no-op stub.
   * @param[in] fullName
   *   Justification: Identifies the target field in `Smp::IComponent::SetSimpleValue`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: No (read-only C-string pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] value
   *   Justification: Supplies the new field value in `Smp::IComponent::SetSimpleValue`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: Value parameter accepted by copy and discarded.
   *   Prerequisites: Valid `Smp::AnySimple` value.
   * @pre `true` (all inputs tolerated safely by stub).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No state change or exception occurs.
   * @containment
   *   Approach: No-op body prevents unguarded string or value access.
   *   Limitations: Silently ignores invalid field names rather than throwing `InvalidFieldName`.
   */
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;

  /**
   * @brief Read an array slice from a named simple array field.
   * @details Stub override; performs no action because `EventManager` exposes no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleArrayValue` interface.
   * @compliance Const-qualified no-op stub leaving caller buffers untouched.
   * @param[in] fullName
   *   Justification: Identifies the array field path in `Smp::IComponent`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Specifies requested element count in `Smp::IComponent`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: Scalar value copied and unused.
   *   Prerequisites: Non-negative `Smp::UInt64` element count.
   * @param[out] values
   *   Justification: Caller output buffer for array elements in `Smp::IComponent`.
   *   Usage: Not written by this stub implementation.
   *   Consumed: No (buffer remains unmodified).
   *   Prerequisites: May be `nullptr` or pre-allocated array of at least `length` items.
   * @param[in] startIndex
   *   Justification: Starting element index in the array field.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: Scalar value copied and unused.
   *   Prerequisites: Defaults to `0`.
   * @pre `true` (stub ignores all parameters safely).
   * @post Caller `values` buffer and `*this` remain unchanged.
   * @error
   *   - None (no-throw stub): Leaves `values` untouched.
   * @containment
   *   Approach: No-op body avoids out-of-bounds buffer writes when no fields exist.
   *   Limitations: Does not populate `values` or report missing field errors.
   */
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;

  /**
   * @brief Write an array slice to a named simple array field.
   * @details Stub override; performs no action because `EventManager` exposes no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleArrayValue` interface.
   * @compliance No-op stub implementing `Smp::IComponent` array setter safely.
   * @param[in] fullName
   *   Justification: Identifies the array field path in `Smp::IComponent`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Specifies element count in `values`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: Scalar value copied and unused.
   *   Prerequisites: Non-negative `Smp::UInt64` count.
   * @param[in] values
   *   Justification: Supplies input array values in `Smp::IComponent`.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: No (input array not dereferenced).
   *   Prerequisites: May be `nullptr` or valid `Smp::AnySimpleArray`.
   * @param[in] startIndex
   *   Justification: Starting element index in the target array field.
   *   Usage: Ignored in this stub implementation.
   *   Consumed: Scalar value copied and unused.
   *   Prerequisites: Defaults to `0`.
   * @pre `true` (stub ignores all parameters safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Performs no memory access or mutation.
   * @containment
   *   Approach: No-op body prevents invalid reads from `values`.
   *   Limitations: Silently discards writes rather than throwing `InvalidFieldName`.
   */
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;

  /**
   * @brief Attempt to add a dynamic child object to a collection.
   * @details Stub override returning `false` because `EventManager` is a leaf service without child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::AddChild` interface.
   * @compliance Safely rejects unsupported child insertion by returning `false`.
   * @param[in] child
   *   Justification: Required by `Smp::IComponent::AddChild` to specify the child object.
   *   Usage: Ignored; `EventManager` supports no child objects.
   *   Consumed: No (non-owning pointer; not stored or modified).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Identifies the target child collection in `Smp::IComponent`.
   *   Usage: Ignored; no collections exist on this component.
   *   Consumed: No (non-owning pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (any pointer combination is safely rejected).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Indicates whether the child object was accepted into the collection.
   *   Usage: Caller checks the boolean return to detect unsupported dynamic child addition.
   *   Consumed: Scalar boolean returned by copy; no ownership transferred.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`; dynamic child composition is unsupported.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard scalar; no cleanup needed.
   *   Specific Values: Always `false` (addition refused).
   *   Error Behavior: Never throws; returns `false` unconditionally.
   * @retval false `EventManager` does not support adding child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Immediately returns `false` without dereferencing inputs or altering collections.
   *   Limitations: Does not distinguish invalid collection pointers from unsupported operations.
   */
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::ICollectionBase *collection) override;

  /**
   * @brief Attempt to remove a dynamic child object from a collection.
   * @details Stub override returning `false` because `EventManager` holds no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::RemoveChild` interface.
   * @compliance Safely rejects unsupported child removal by returning `false`.
   * @param[in] child
   *   Justification: Required by `Smp::IComponent::RemoveChild` to specify the child object.
   *   Usage: Ignored; no child objects are held.
   *   Consumed: No (non-owning pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Identifies the target child collection in `Smp::IComponent`.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (non-owning pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (any pointer combination is safely rejected).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Indicates whether the child object was removed from the collection.
   *   Usage: Caller branches on the boolean to verify removal status.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`; no child collections exist.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard scalar; no cleanup needed.
   *   Specific Values: Always `false` (removal not performed).
   *   Error Behavior: Never throws; returns `false` unconditionally.
   * @retval false `EventManager` owns no child objects to remove.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Stateless `false` return prevents dangling pointer or collection access.
   *   Limitations: Cannot report per-argument validation diagnostics.
   */
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;

  /**
   * @brief Check whether a named child resides in a given collection.
   * @details Stub override returning `nullptr` because `EventManager` contains no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::IsChildInCollection` interface.
   * @compliance Const-qualified lookup returning `nullptr` when no children exist.
   * @param[in] child
   *   Justification: Identifies the child name to locate within `collection`.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (read-only string pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] collection
   *   Justification: Identifies the collection to search.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (non-owning pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IObject *
   *   Justification: Returns the matching child object pointer if present, or `nullptr`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`; child containment is unsupported.
   *   Lifecycle: Not applicable (returns `nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; always returns `nullptr`.
   * @retval nullptr Child is not in any collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids unsafe pointer traversal.
   *   Limitations: Does not differentiate invalid collection arguments from empty results.
   */
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

  /**
   * @brief Retrieve a child object by name.
   * @details Stub override returning `nullptr` because `EventManager` has no child objects.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetChild` interface.
   * @compliance Const-qualified override returning `nullptr` for empty child hierarchy.
   * @param[in] name
   *   Justification: Specifies the child object name requested by the caller.
   *   Usage: Ignored; no child objects are managed.
   *   Consumed: No (read-only string pointer; not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IObject *
   *   Justification: Provides the requested child object or `nullptr` if absent.
   *   Usage: Caller validates non-null before using the returned pointer.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must handle `nullptr`.
   *   Limitations: Always returns `nullptr`; `EventManager` is a leaf service.
   *   Lifecycle: Not applicable (returns `nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr No child matches `name`.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless return of `nullptr` avoids null-string dereference.
   *   Limitations: Child lookup always yields `nullptr`.
   */
  Smp::IObject *GetChild(Smp::String8 name) const override;

  // IEventManager methods

  /**
   * @brief Query the event ID for a given event name.
   * @details Looks up `eventName` in `_eventIds` under `_mutex` (1-second timed lock).
   * If no matching event exists, allocates a new unique `EventId` from `_nextEventId++`
   * and records bidirectional mappings in `_eventIds` and `_eventNames`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IEventManager::QueryEventId` interface.
   * @compliance Enforces null-pointer guard [CS-0010.15], timed mutex locking [CS-0010.26, CS-0010.46],
   * and explicit iterator types without `auto` [CS-0010.34].
   * @param[in] eventName
   *   Justification: Supplies the symbolic SMP event name whose unique numeric ID is requested.
   *   Usage: Validated against `nullptr`, looked up in `_eventIds`, and stored as a `std::string` key if new.
   *   Consumed: No (read-only C-string pointer; copied into internal `std::string` when newly registered).
   *   Prerequisites: `eventName != nullptr` and must point to a valid null-terminated UTF-8/ASCII string.
   * @pre `eventName != nullptr`
   * @post Returned `EventId` is stably mapped to `eventName` in `_eventIds` for subsequent calls.
   * @invariant `_mutex` guards all reads and insertions across `_eventIds`, `_eventNames`, and `_nextEventId`.
   * @return Smp::Services::EventId
   *   Justification: Provides the compact integer identifier required for `Subscribe`, `Unsubscribe`, and `Emit`.
   *   Usage: Caller passes the returned `EventId` to subscription and emission methods.
   *   Consumed: Scalar integer returned by copy; caller owns its copy.
   *   Prerequisites: Valid non-null `eventName` supplied on entry.
   *   Limitations: Dynamically allocated IDs grow monotonically (`>= 20`) and are never reclaimed.
   *   Lifecycle: Valid for the lifetime of this `EventManager` instance.
   *   Modification / Deletion: Caller owns scalar copy and may discard freely; no deletion required.
   *   Specific Values: Predefined SMP IDs (`1..19`) for standard events, or dynamically assigned IDs `>= 20`.
   *   Error Behavior: Throws before returning if `eventName` is null, lock times out, or map allocation fails.
   * @throws core::InvalidEventName [Validation / Contract] Thrown before locking if `eventName == nullptr`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if inserting a new event name fails.
   * @error
   *   - Validation / Contract (`eventName == nullptr`): Throws `core::InvalidEventName`; strong guarantee (`*this` unchanged).
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds via RAII lock release; basic guarantee if `_eventIds` succeeded before `_eventNames`.
   * @containment
   *   Approach: Null check executes prior to locking; `std::unique_lock<std::timed_mutex>` bounds wait time to 1s
   *   and guarantees RAII unlock across all exception paths.
   *   Limitations: If `_eventNames[newId]` throws after `_eventIds[eventName] = newId`, `_eventIds` retains the new entry;
   *   does not reject empty `""` event names.
   */
  Smp::Services::EventId QueryEventId(Smp::String8 eventName) override;

  /**
   * @brief Subscribe an entry point to an event.
   * @details Validates `entryPoint`, acquires `_mutex` with a 1-second timeout, verifies
   * the entry point is not already present in `_subscriptions[event]`, and appends it.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IEventManager::Subscribe` interface.
   * @compliance Checks null pointer before locking, uses RAII timed mutex, prevents duplicate subscription,
   * and avoids `auto`.
   * @param[in] event
   *   Justification: Identifies the target global event to which `entryPoint` is attached.
   *   Usage: Used as lookup/insertion key in `_subscriptions`.
   *   Consumed: No (scalar value copied for map lookup).
   *   Prerequisites: Any valid `Smp::Services::EventId` (typically obtained from `QueryEventId` or SMP predefined IDs).
   * @param[in] entryPoint
   *   Justification: Supplies the callback object to execute when `event` is emitted.
   *   Usage: Checked against `nullptr`, searched for duplicates, and stored as a non-owning pointer in `_subscriptions[event]`.
   *   Consumed: No (pointer is retained non-owningly; caller retains lifetime ownership).
   *   Prerequisites: `entryPoint != nullptr`; pointee must remain valid until unsubscribed or `EventManager` destruction.
   * @pre `entryPoint != nullptr` and `entryPoint` is not already subscribed to `event`.
   * @post `entryPoint` appears once at the end of `_subscriptions[event]`.
   * @invariant `_subscriptions[event]` contains no duplicate `const Smp::IEntryPoint *` pointers.
   * @throws std::invalid_argument [Validation / Contract] Thrown before locking if `entryPoint == nullptr`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @throws core::EntryPointAlreadySubscribed [Validation / Contract] Thrown if `entryPoint` is already registered for `event`.
   * @error
   *   - Validation / Contract (`entryPoint == nullptr`): Throws `std::invalid_argument`; strong guarantee (`*this` unchanged).
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   *   - Validation / Contract (duplicate subscription): Throws `core::EntryPointAlreadySubscribed`; strong guarantee (subscription list unchanged, though empty map entry may exist).
   *   - Resource / Allocation (`std::bad_alloc`): Vector growth failure unwinds cleanly via RAII lock release.
   * @containment
   *   Approach: Pre-lock null guard and duplicate check prevent corrupting subscriber lists; `std::unique_lock`
   *   bounds lock waiting to 1s and unlocks automatically on throw.
   *   Limitations: Calling `Subscribe` for a nonexistent `event` key default-constructs an empty vector entry in `_subscriptions`;
   *   cannot detect dangling `entryPoint` pointers if caller destroys the pointee without calling `Unsubscribe`.
   */
  void Subscribe(Smp::Services::EventId event,
                 const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Unsubscribe an entry point from an event.
   * @details Validates `entryPoint`, acquires `_mutex` with a 1-second timeout, locates
   * `event` and `entryPoint` in `_subscriptions`, and erases the entry.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IEventManager::Unsubscribe` interface.
   * @compliance Enforces null pointer validation, timed mutex exclusion, and explicit container iterators.
   * @param[in] event
   *   Justification: Identifies the event whose subscriber list should be modified.
   *   Usage: Searched as key in `_subscriptions`.
   *   Consumed: No (scalar value copied for map lookup).
   *   Prerequisites: Must correspond to an event with active subscriptions containing `entryPoint`.
   * @param[in] entryPoint
   *   Justification: Identifies the specific subscriber pointer to detach from `event`.
   *   Usage: Checked against `nullptr` and matched by pointer equality inside `_subscriptions[event]`.
   *   Consumed: No (pointer is only compared for removal; caller retains pointee ownership).
   *   Prerequisites: `entryPoint != nullptr` and currently subscribed to `event`.
   * @pre `entryPoint != nullptr` and `entryPoint` is currently subscribed to `event`.
   * @post `entryPoint` is no longer present in `_subscriptions[event]`.
   * @invariant Remaining subscribers preserve relative registration order under `_mutex`.
   * @throws std::invalid_argument [Validation / Contract] Thrown before locking if `entryPoint == nullptr`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @throws core::EntryPointNotSubscribed [Validation / Contract] Thrown if `event` has no subscribers or `entryPoint` is not subscribed.
   * @error
   *   - Validation / Contract (`entryPoint == nullptr`): Throws `std::invalid_argument`; strong guarantee (`*this` unchanged).
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   *   - Validation / Contract (not subscribed): Throws `core::EntryPointNotSubscribed`; strong guarantee (`_subscriptions` unchanged).
   * @containment
   *   Approach: Validates arguments and membership before erasing; `std::unique_lock<std::timed_mutex>` enforces 1s wait limit
   *   and automatic unlock on exception.
   *   Limitations: Cannot be safely awaited beyond 1s under heavy lock contention; leaves empty vector entry in `_subscriptions`
   *   when the final subscriber is removed.
   */
  void Unsubscribe(Smp::Services::EventId event,
                   const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Emit an event.
   * @details Snapshots the subscriber vector for `event` under `_mutex` (1-second timed lock),
   * releases the lock, and invokes `Execute()` on each non-null entry point in registration order.
   * Copying the subscriber list avoids re-entrant deadlocks if callbacks subscribe or unsubscribe.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0050.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IEventManager::Emit` interface.
   * @compliance Snapshots subscribers under RAII timed lock before callback dispatch to prevent lock-order deadlocks.
   * @param[in] event
   *   Justification: Identifies which global event's subscribers should be invoked.
   *   Usage: Looked up in `_subscriptions` under `_mutex`.
   *   Consumed: No (scalar value copied for map lookup).
   *   Prerequisites: Any `Smp::Services::EventId`; unknown IDs result in a safe no-op.
   * @param[in] synchronous
   *   Justification: Required by `Smp::Services::IEventManager::Emit` to indicate synchronous execution mode.
   *   Usage: Accepted for interface compliance; current implementation executes subscribers synchronously inline.
   *   Consumed: No (scalar boolean copied; defaults to `true`).
   *   Prerequisites: `true` or `false` (`Smp::Bool`).
   * @pre All subscribed `IEntryPoint *` pointers for `event` remain valid during execution.
   * @post All non-null subscribers present in the snapshot were invoked unless an entry point threw.
   * @invariant `_mutex` is never held while calling `ep->Execute()`, preventing re-entrant self-deadlock.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if copying the subscriber list fails.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged, no callbacks run).
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds cleanly via RAII lock release before any callback runs.
   *   - Callback / User Fault (exception thrown by `ep->Execute()`): Propagates immediately to caller; remaining subscribers in snapshot are skipped.
   * @containment
   *   Approach: Lock scope is strictly bounded to subscriber vector copy; null-checks `ep` before `Execute()`.
   *   Limitations: Exceptions thrown by an entry point abort remaining subscriber notifications; `synchronous == false`
   *   still executes synchronously and [FE-0070.4.10] state-transition re-entry suppression is not enforced.
   */
  void Emit(Smp::Services::EventId event,
            Smp::Bool synchronous = true) override;

private:
  /**
   * @brief Next available event ID for dynamic creation.
   * @details Initialized to `20` so dynamically queried events never collide with
   * SMP predefined event identifiers (`1..19`).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized and guarded by `_mutex` during `QueryEventId`.
   */
  Smp::Services::EventId _nextEventId = 20;

  /**
   * @brief Mapping from event name to event ID.
   * @details Stores predefined SMP event names and dynamically registered event names
   * mapped to their unique `Smp::Services::EventId`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Accessed exclusively while holding `_mutex` (except during constructor initialization).
   */
  std::map<std::string, Smp::Services::EventId> _eventIds;

  /**
   * @brief Mapping from event ID to event name.
   * @details Reverse lookup table populated for dynamically registered event identifiers
   * in `QueryEventId`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Accessed exclusively under `_mutex` exclusion.
   */
  std::map<Smp::Services::EventId, std::string> _eventNames;

  /**
   * @brief Subscription list for each event ID.
   * @details Maps each active `EventId` to an ordered vector of non-owning subscribed
   * `const Smp::IEntryPoint *` pointers.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance All reads, copies, insertions, and erasures occur strictly under `_mutex`.
   */
  std::map<Smp::Services::EventId, std::vector<const Smp::IEntryPoint *>> _subscriptions;

  /**
   * @brief Mutex for thread-safe access.
   * @details Timed mutex bounding lock acquisition waits across `QueryEventId`, `Subscribe`,
   * `Unsubscribe`, and `Emit` snapshot operations.
   *
   * @task TSK-20261113-003
   * @feature FE-0030, FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Enforces bounded timed locking and single-mutex serialization for all shared fields.
   */
  mutable std::timed_mutex _mutex;

  /**
   * @brief Register standard SMP predefined events.
   * @details Populates `_eventIds` with the 19 standard SMP global simulation lifecycle
   * and time-change event names and their reserved identifiers. Invoked solely from `EventManager()`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0090.1, CS-0090.2, CS-0090.8
   * @compliance Deterministic private helper called once during construction before concurrent exposure.
   * @post All 19 predefined SMP global events are present in `_eventIds`.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if map node allocation fails.
   * @error
   *   - Resource / Allocation (`std::bad_alloc`): Construction aborts and RAII destroys partially populated maps.
   * @containment
   *   Approach: Runs strictly inside constructor prior to publishing `this` to any external thread.
   *   Limitations: Cannot recover from out-of-memory failures during map insertion.
   */
  void RegisterPredefinedEvents();
};

} // namespace utils
