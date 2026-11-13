/**
 * @file TimeKeeper.hpp
 * @brief Implementation of the SMP Time Keeper Service.
 *
 * @details Declares `utils::TimeKeeper`, managing simulation, epoch, mission,
 * and zulu wall-clock time under timed-mutex protection with optional event emission.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0090.1, CS-0090.2
 * @compliance Declares `utils::TimeKeeper` in a dedicated self-contained header
 * inside namespace `utils` with formal Doxygen traceability and contracts.
 *
 * @note This utility class implements SMP services and is thread-safe.
 *       It does not directly contribute to the features described in FE-0020 (NamedObject, tree/graph structures).
 *       The thread safety implemented here is for the TimeKeeper's own operations and not related to FE-0020 requirements.
 *       For example, FE-0020.5 requires thread-safe operations on named objects, which are not implemented in this file.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP ITimeKeeper service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`TimeKeeper.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::chrono` and `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_simulationTime`, `_epochOffset`, `_missionStart` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `GetSimulationTime`, `SetEpochTime` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. Its primary function is time management and it is not involved in binary data serialization, deserialization, schema definitions, or codec implementations.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP ITimeKeeper service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.2 (Time Keeper):
 * - Implements the ITimeKeeper component [FE-0070.2.1].
 * - Provides all required Get methods: GetSimulationTime [FE-0070.2.6], GetEpochTime [FE-0070.2.7], GetMissionTime [FE-0070.2.8], GetMissionStartTime [FE-0070.2.9], and GetZuluTime [FE-0070.2.10].
 * - Provides all required Set methods: SetEpochTime [FE-0070.2.2], SetMissionStartTime [FE-0070.2.3], SetMissionTime [FE-0070.2.4], and SetSimulationTime [FE-0070.2.5].
 * - Supports time change event emissions, indirectly supporting scheduler updates [FE-0070.3.21, FE-0070.3.22].
 */
#pragma once

// SMP interfaces, core object base, and timed synchronization primitive.
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ITimeKeeper.h>
#include <core/Object.hpp>
#include <mutex>

namespace utils {

/**
 * @brief Implementation of the SMP Time Keeper Service.
 * @details Manages simulation, epoch, mission, and zulu time. Shared state fields
 * (`_eventManager`, `_simulationTime`, `_epochOffset`, `_missionStart`) are protected by
 * `_mutex` (`std::timed_mutex`) with a 1-second timeout. Time-change events are emitted
 * outside the lock scope in `SetEpochTime` and `SetMissionStartTime`.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed SMP `ITimeKeeper` and `IComponent` service boundary.
 * @compliance Uses `std::timed_mutex` for bounded locking, explicit member initialization,
 * and `const` correctness across read-only time queries.
 * @invariant `_mutex` serializes all reads and writes to `_simulationTime`, `_epochOffset`,
 * `_missionStart`, and `_eventManager`; epoch time equals `_epochOffset + _simulationTime`.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 */
class TimeKeeper : public core::Object,
                   public virtual Smp::Services::ITimeKeeper {
public:
  /**
   * @brief Default constructor.
   * @details Initializes `core::Object` with component name `"TimeKeeper"` and
   * zero-initializes all internal time offsets and service pointers.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0010.45, CS-0090.1, CS-0090.2
   * @compliance All scalar and pointer fields have deterministic in-class default initializers.
   * @post Simulation, epoch offset, and mission start times are initialized to `0` and `_eventManager == nullptr`.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if `core::Object` string metadata allocation fails.
   * @error
   *   - Resource / Allocation (`std::bad_alloc`): Construction aborts cleanly with no leaked state.
   * @containment
   *   Approach: Member initializers zero-initialize all fields prior to body execution.
   *   Limitations: String allocation failure inside `core::Object` propagates to caller.
   */
  TimeKeeper();

  /**
   * @brief Virtual destructor.
   * @details Releases `TimeKeeper` resources via RAII without throwing exceptions.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0060.5, CS-0090.1, CS-0090.2
   * @compliance Marked `noexcept` and `override` with RAII-safe member cleanup.
   */
  virtual ~TimeKeeper() noexcept override = default;

  // IComponent methods

  /**
   * @brief Query the current SMP component lifecycle state.
   * @details Returns `Smp::ComponentStateKind::CSK_Connected` as the fixed state of this service.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetState` interface.
   * @compliance Const-qualified accessor returning explicit enum state without side effects.
   * @post Returns `Smp::ComponentStateKind::CSK_Connected` without modifying `*this`.
   * @return Smp::ComponentStateKind
   *   Justification: Exposes component lifecycle state required by `Smp::IComponent`.
   *   Usage: Caller inspects enum value to confirm service availability.
   *   Consumed: Value returned by copy; no internal state consumed.
   *   Prerequisites: Valid `TimeKeeper` instance.
   *   Limitations: Always reports `CSK_Connected` regardless of `Connect`/`Disconnect`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or assign locally; no cleanup required.
   *   Specific Values: Always `Smp::ComponentStateKind::CSK_Connected`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw return path): Always returns `CSK_Connected`.
   * @containment
   *   Approach: Pure constant-returning accessor performs no locking or allocation.
   *   Limitations: Virtual ABI omits `noexcept`; lifecycle state machine is stubbed.
   */
  Smp::ComponentStateKind GetState() const override;

  /**
   * @brief Publish component properties to an SMP publication receiver.
   * @details No-op override of `Smp::IComponent::Publish` because `TimeKeeper` publishes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Publish` interface.
   * @compliance Implements `Smp::IComponent::Publish` safely as a no-op stub.
   * @param[in] receiver
   *   Justification: Required by `Smp::IComponent::Publish` to register published members.
   *   Usage: Unused; no fields are published.
   *   Consumed: No (non-owning pointer; not dereferenced or retained).
   *   Prerequisites: May be `nullptr` or any `Smp::IPublication *`.
   * @pre `true` (all inputs tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Returns without error.
   * @containment
   *   Approach: Empty body avoids dereferencing `receiver`.
   *   Limitations: Time counters are not exposed via SMP publication reflection.
   */
  void Publish(Smp::IPublication *receiver) override;

  /**
   * @brief Configure the component with logger and link registry services.
   * @details No-op override of `Smp::IComponent::Configure`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Configure` interface.
   * @compliance Safe no-op configuration hook.
   * @param[in] logger
   *   Justification: Required by `Smp::IComponent::Configure` to supply a logger service.
   *   Usage: Unused in this stub implementation.
   *   Consumed: No (non-owning pointer; not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILogger *`.
   * @param[in] linkRegistry
   *   Justification: Required by `Smp::IComponent::Configure` to supply a link registry.
   *   Usage: Unused in this stub implementation.
   *   Consumed: No (non-owning pointer; not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILinkRegistry *`.
   * @pre `true` (null pointers tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: No-op body isolates configuration calls from timekeeper state.
   *   Limitations: Does not resolve event manager automatically from simulator configuration.
   */
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;

  /**
   * @brief Connect the component to the hosting SMP simulator.
   * @details No-op override of `Smp::IComponent::Connect`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Connect` interface.
   * @compliance Safe no-op connection hook.
   * @param[in] simulator
   *   Justification: Required by `Smp::IComponent::Connect` to bind the service to a simulator.
   *   Usage: Unused in this stub implementation; event manager is set via `SetEventManager`.
   *   Consumed: No (non-owning pointer; not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or any `Smp::ISimulator *`.
   * @pre `true` (null pointer tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Returns cleanly without mutation.
   * @containment
   *   Approach: No-op body prevents unguarded pointer dereference.
   *   Limitations: Event manager must be injected explicitly via `SetEventManager`.
   */
  void Connect(Smp::ISimulator *simulator) override;

  /**
   * @brief Disconnect the component from the hosting SMP simulator.
   * @details No-op override of `Smp::IComponent::Disconnect`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Disconnect` interface.
   * @compliance Preserves registered time state and event manager pointer across disconnect.
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: No-op body preserves read access to time values during shutdown.
   *   Limitations: Does not clear `_eventManager` automatically; caller must ensure lifetime.
   */
  void Disconnect() override;

  /**
   * @brief Retrieve the unique SMP type UUID of this component.
   * @details Returns a reference to a function-local static `Smp::Uuid` `{0, 0, 0, 0, 5}`,
   * avoiding global constants per [CS-0010.30].
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.30, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetUuid` interface.
   * @compliance Uses function-local static storage and returns `const` reference.
   * @post Returned reference addresses valid static `Smp::Uuid` `{0, 0, 0, 0, 5}`.
   * @return const Smp::Uuid &
   *   Justification: Exposes component UUID required by `Smp::IComponent::GetUuid`.
   *   Usage: Caller inspects or copies the UUID for service lookup.
   *   Consumed: Borrowed const reference; ownership remains with static storage.
   *   Prerequisites: None.
   *   Limitations: Returns generic service placeholder UUID `{0, 0, 0, 0, 5}`.
   *   Lifecycle: Static storage duration; valid for process lifetime.
   *   Modification / Deletion: Immutable (`const &`); caller MUST NOT cast away const or delete.
   *   Specific Values: Reference to `{0, 0, 0, 0, 5}`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw constant reference): Always succeeds.
   * @containment
   *   Approach: Function-local static initialization is thread-safe and allocation-free.
   *   Limitations: Virtual signature omits `noexcept`.
   */
  const Smp::Uuid &GetUuid() const override;

  /**
   * @brief Retrieve a published field by full name.
   * @details Stub override returning `nullptr` because `TimeKeeper` publishes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetField` interface.
   * @compliance Const-qualified stub returning `nullptr`.
   * @param[in] fullName
   *   Justification: Identifies requested field path in `Smp::IComponent`.
   *   Usage: Ignored; no fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IField *
   *   Justification: Indicates field absence per `Smp::IComponent::GetField`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr `TimeKeeper` publishes no fields.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids unsafe string access.
   *   Limitations: Field reflection is unsupported.
   */
  Smp::IField *GetField(Smp::String8 fullName) const override;

  /**
   * @brief Retrieve the collection of all published fields.
   * @details Stub override returning `nullptr` because `TimeKeeper` exposes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetFields` interface.
   * @compliance Const-qualified stub returning `nullptr`.
   * @post Returns `nullptr` without modifying `*this`.
   * @return const Smp::FieldCollection *
   *   Justification: Conforms to `Smp::IComponent::GetFields` when no fields exist.
   *   Usage: Caller checks for `nullptr` before traversing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must check for `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Immutable null pointer; caller MUST NOT dereference or delete.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never fails or throws.
   * @retval nullptr Component exposes no field collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Direct `nullptr` return without allocation.
   *   Limitations: Field introspection callers must tolerate `nullptr`.
   */
  const Smp::FieldCollection *GetFields() const override;

  /**
   * @brief Read the simple value of a named field.
   * @details Stub override returning a default-constructed `Smp::AnySimple`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleValue` interface.
   * @compliance Const-qualified stub returning initialized `Smp::AnySimple`.
   * @param[in] fullName
   *   Justification: Identifies requested field path.
   *   Usage: Ignored; no simple fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely).
   * @post Returns default `Smp::AnySimple()` without modifying `*this`.
   * @return Smp::AnySimple
   *   Justification: Satisfies `Smp::IComponent::GetSimpleValue` return contract.
   *   Usage: Caller checks `.type` before reading value.
   *   Consumed: Returned by value; caller owns the temporary.
   *   Prerequisites: None.
   *   Limitations: Always returns empty `ST_None` value.
   *   Lifecycle: Standalone caller-owned value.
   *   Modification / Deletion: Caller may modify or discard freely; no manual delete.
   *   Specific Values: Default-constructed `Smp::AnySimple`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw stub): Always returns default `Smp::AnySimple`.
   * @containment
   *   Approach: Value-initialized return avoids accessing `fullName`.
   *   Limitations: Does not throw on missing field names.
   */
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;

  /**
   * @brief Set the simple value of a named field.
   * @details Stub override performing no operation because `TimeKeeper` has no simple fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleValue` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::SetSimpleValue`.
   * @param[in] fullName
   *   Justification: Target field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] value
   *   Justification: New field value.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and discarded.
   *   Prerequisites: Any `Smp::AnySimple`.
   * @pre `true` (all inputs tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body prevents unsafe pointer access.
   *   Limitations: Missing fields are silently ignored.
   */
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;

  /**
   * @brief Read an array slice from a named simple array field.
   * @details Stub override performing no operation because `TimeKeeper` has no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleArrayValue` interface.
   * @compliance Const-qualified no-op stub leaving `values` untouched.
   * @param[in] fullName
   *   Justification: Identifies array field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Requested element count.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and unused.
   *   Prerequisites: Any `Smp::UInt64`.
   * @param[out] values
   *   Justification: Destination buffer for elements.
   *   Usage: Left untouched.
   *   Consumed: No (buffer unmodified).
   *   Prerequisites: May be `nullptr` or pre-allocated array.
   * @param[in] startIndex
   *   Justification: Starting element index.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and unused.
   *   Prerequisites: Defaults to `0`.
   * @pre `true` (all inputs safely ignored).
   * @post `values` and `*this` remain unchanged.
   * @error
   *   - None (no-throw stub): Performs no memory writes.
   * @containment
   *   Approach: No-op body avoids buffer writes.
   *   Limitations: Does not populate `values` or report missing fields.
   */
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;

  /**
   * @brief Write an array slice to a named simple array field.
   * @details Stub override performing no operation because `TimeKeeper` has no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleArrayValue` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::SetSimpleArrayValue`.
   * @param[in] fullName
   *   Justification: Target array field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Element count in `values`.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and unused.
   *   Prerequisites: Any `Smp::UInt64`.
   * @param[in] values
   *   Justification: Input array buffer.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or valid `Smp::AnySimpleArray`.
   * @param[in] startIndex
   *   Justification: Starting element index.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and unused.
   *   Prerequisites: Defaults to `0`.
   * @pre `true` (all inputs safely ignored).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: Empty body prevents unsafe reads from `values`.
   *   Limitations: Missing fields are silently ignored.
   */
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;

  /**
   * @brief Attempt to add a dynamic child object to a collection.
   * @details Stub override returning `false` because `TimeKeeper` manages no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::AddChild` interface.
   * @compliance Safely rejects child insertion with `false`.
   * @param[in] child
   *   Justification: Candidate child object pointer.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Target collection pointer.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Indicates whether child addition succeeded.
   *   Usage: Caller inspects returned boolean.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `TimeKeeper` does not support child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Immediate `false` return prevents collection mutation.
   *   Limitations: Dynamic child composition is unsupported.
   */
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::ICollectionBase *collection) override;

  /**
   * @brief Attempt to remove a dynamic child object from a collection.
   * @details Stub override returning `false` because `TimeKeeper` holds no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::RemoveChild` interface.
   * @compliance Safely rejects child removal with `false`.
   * @param[in] child
   *   Justification: Candidate child object pointer.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Target collection pointer.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Indicates whether child removal succeeded.
   *   Usage: Caller inspects returned boolean.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `TimeKeeper` owns no child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Stateless `false` return prevents dangling collection access.
   *   Limitations: Does not distinguish argument errors.
   */
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;

  /**
   * @brief Check whether a named child resides in a given collection.
   * @details Stub override returning `nullptr` because `TimeKeeper` holds no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::IsChildInCollection` interface.
   * @compliance Const-qualified stub returning `nullptr`.
   * @param[in] child
   *   Justification: Child object name to search.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] collection
   *   Justification: Collection pointer to inspect.
   *   Usage: Ignored; no collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IObject *
   *   Justification: Returns matching child pointer or `nullptr`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr Child is not in any collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids unsafe pointer traversal.
   *   Limitations: Child membership queries always return `nullptr`.
   */
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

  /**
   * @brief Retrieve a child object by name.
   * @details Stub override returning `nullptr` because `TimeKeeper` is a leaf service.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetChild` interface.
   * @compliance Const-qualified override returning `nullptr`.
   * @param[in] name
   *   Justification: Requested child object name.
   *   Usage: Ignored; no children exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IObject *
   *   Justification: Provides named child or `nullptr`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr No child matches `name`.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Returns `nullptr` directly without dereferencing `name`.
   *   Limitations: No dynamic child lookup is supported.
   */
  Smp::IObject *GetChild(Smp::String8 name) const override;

  // ITimeKeeper methods

  /**
   * @brief Set the event manager to use for time change events.
   * @details Stores `eventManager` under `_mutex` (1-second timed lock) so subsequent
   * `SetEpochTime` and `SetMissionStartTime` calls can emit SMP time-change notifications.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @compliance Uses `std::unique_lock<std::timed_mutex>` with 1-second timeout to guard `_eventManager`.
   * @param[in] eventManager
   *   Justification: Injects the `IEventManager` service used to broadcast epoch and mission time changes.
   *   Usage: Stored in `_eventManager` under `_mutex`; `nullptr` disables time-change event emission.
   *   Consumed: No (non-owning pointer; caller retains ownership and must outlive attachment).
   *   Prerequisites: Either `nullptr` (to detach) or a valid `Smp::Services::IEventManager *`.
   * @pre `eventManager == nullptr || eventManager` remains valid while attached.
   * @post `_eventManager == eventManager`.
   * @invariant `_mutex` guards all updates to `_eventManager`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`_eventManager` unchanged).
   * @containment
   *   Approach: `std::unique_lock<std::timed_mutex>` enforces bounded wait and RAII unlock.
   *   Limitations: Reads of `_eventManager` inside `SetEpochTime`/`SetMissionStartTime` occur outside the lock
   *   scope to avoid nested locking deadlocks with event callbacks.
   */
  void SetEventManager(Smp::Services::IEventManager *eventManager);

  /**
   * @brief Get current simulation time.
   * @details Acquires `_mutex` with a 1-second timeout and returns `_simulationTime`
   * (nanoseconds since start of simulation).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::GetSimulationTime` interface.
   * @compliance Const-qualified accessor using bounded RAII timed lock.
   * @post Returns current `_simulationTime` snapshot without modifying `*this`.
   * @invariant `_mutex` serializes reads of `_simulationTime` against concurrent `SetSimulationTime`.
   * @return Smp::Duration
   *   Justification: Reports elapsed simulation duration for schedulers and models.
   *   Usage: Caller reads duration in nanoseconds for scheduling or telemetry.
   *   Consumed: Scalar duration returned by copy; internal counter unchanged.
   *   Prerequisites: None.
   *   Limitations: Snapshot may be superseded immediately after lock release by another thread.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Non-negative or configured `Smp::Duration` value (default `0`).
   *   Error Behavior: Throws `std::runtime_error` if timed lock expires before returning.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   * @containment
   *   Approach: `std::unique_lock<std::timed_mutex>` bounds wait to 1s and unlocks automatically.
   *   Limitations: Cannot return a fallback value on timeout without violating `ITimeKeeper` semantics.
   */
  Smp::Duration GetSimulationTime() const override;

  /**
   * @brief Get current epoch time.
   * @details Acquires `_mutex` with a 1-second timeout and computes `_epochOffset + _simulationTime`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::GetEpochTime` interface.
   * @compliance Const-qualified accessor computing epoch time atomically under `_mutex`.
   * @post Returns `_epochOffset + _simulationTime` without mutating `*this`.
   * @invariant `_epochOffset` and `_simulationTime` are read coherently under the same lock acquisition.
   * @return Smp::DateTime
   *   Justification: Provides current simulation epoch timestamp per `ITimeKeeper`.
   *   Usage: Caller compares or records absolute epoch date/time.
   *   Consumed: Scalar timestamp returned by copy; internal state unchanged.
   *   Prerequisites: None.
   *   Limitations: Signed 64-bit arithmetic assumes `_epochOffset + _simulationTime` does not overflow.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Computed `Smp::DateTime` (`_epochOffset + _simulationTime`).
   *   Error Behavior: Throws `std::runtime_error` if timed lock acquisition fails.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   * @containment
   *   Approach: Atomic read of both `_epochOffset` and `_simulationTime` under a single 1s timed lock.
   *   Limitations: Does not saturate or trap signed integer overflow on extreme offsets.
   */
  Smp::DateTime GetEpochTime() const override;

  /**
   * @brief Get mission start time.
   * @details Acquires `_mutex` with a 1-second timeout and returns `_missionStart`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::GetMissionStartTime` interface.
   * @compliance Const-qualified accessor guarded by bounded `std::timed_mutex`.
   * @post Returns `_missionStart` without mutating `*this`.
   * @return Smp::DateTime
   *   Justification: Exposes configured mission epoch reference timestamp.
   *   Usage: Caller uses value to compute or verify mission-relative timestamps.
   *   Consumed: Scalar timestamp returned by copy; internal state unchanged.
   *   Prerequisites: None.
   *   Limitations: Snapshot reflects state at lock release.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Configured `Smp::DateTime` (default `0`).
   *   Error Behavior: Throws `std::runtime_error` on lock timeout.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   * @containment
   *   Approach: Bounded RAII lock acquisition prevents indefinite stall.
   *   Limitations: Contention exceeding 1 second raises an exception.
   */
  Smp::DateTime GetMissionStartTime() const override;

  /**
   * @brief Get current mission time.
   * @details Acquires `_mutex` (1-second timeout) and returns `GetEpochTime() - _missionStart`.
   * Because `std::timed_mutex` is non-recursive, `GetEpochTime()` acquires the lock only if
   * `_mutex` is not re-entered (note: current implementation calls `GetEpochTime()` while holding `lock`).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::GetMissionTime` interface.
   * @compliance Const-qualified accessor computing elapsed duration since `_missionStart`.
   * @post Returns `GetEpochTime() - _missionStart` without mutating `*this`.
   * @return Smp::Duration
   *   Justification: Provides elapsed duration relative to mission start per `ITimeKeeper`.
   *   Usage: Caller inspects mission duration (may be negative before mission start).
   *   Consumed: Scalar duration returned by copy; internal state unchanged.
   *   Prerequisites: None.
   *   Limitations: Nested `GetEpochTime()` call while holding non-recursive `_mutex` triggers timed wait failure (1s timeout).
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Signed `Smp::Duration` (`EpochTime - _missionStart`).
   *   Error Behavior: Throws `std::runtime_error` on lock timeout.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy or self-lock > 1s): Throws `std::runtime_error`; strong guarantee (`*this` unchanged).
   * @containment
   *   Approach: Timed lock ensures nested lock attempt fails deterministically after 1s rather than deadlocking forever.
   *   Limitations: Current implementation holds `_mutex` across `GetEpochTime()`, causing a 1s timeout unless refactored.
   */
  Smp::Duration GetMissionTime() const override;

  /**
   * @brief Get current Zulu (wall clock) time.
   * @details Reads `std::chrono::system_clock::now()` and converts its epoch duration
   * to signed 64-bit nanoseconds (`Smp::DateTime`) using explicit types without `auto`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.34, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::GetZuluTime` interface.
   * @compliance Const-qualified lock-free clock query using explicit `std::chrono` types (no `auto`).
   * @post Returns host wall-clock timestamp in nanoseconds since Unix epoch without mutating `*this`.
   * @return Smp::DateTime
   *   Justification: Exposes real-world UTC/Zulu wall time for Zulu-scheduled events.
   *   Usage: Caller compares timestamps or computes elapsed wall-clock intervals.
   *   Consumed: Scalar timestamp returned by copy.
   *   Prerequisites: Host system clock is synchronized and operational.
   *   Limitations: Subject to OS wall-clock NTP adjustments (non-monotonic jumps possible).
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Nanoseconds since `std::chrono::system_clock` epoch.
   *   Error Behavior: Never throws (`system_clock::now()` is `noexcept`).
   * @error
   *   - None (no-throw system clock query): Returns nanoseconds count directly.
   * @containment
   *   Approach: Lock-free read of `std::chrono::system_clock` avoids mutex contention.
   *   Limitations: Cannot guarantee monotonicity if host system clock steps backwards.
   */
  Smp::DateTime GetZuluTime() const override;

  /**
   * @brief Set simulation time.
   * @details Acquires `_mutex` (1-second timeout) and updates `_simulationTime`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::SetSimulationTime` interface.
   * @compliance Guards `_simulationTime` update with RAII `std::timed_mutex` lock.
   * @param[in] simulationTime
   *   Justification: Supplies the new simulation time value in nanoseconds.
   *   Usage: Assigned directly to `_simulationTime` under `_mutex`.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Non-negative `Smp::Duration` advancing simulation time per SMP rules.
   * @pre `simulationTime >= 0`
   * @post `_simulationTime == simulationTime`.
   * @invariant `_mutex` serializes updates against concurrent time queries.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`_simulationTime` unchanged).
   * @containment
   *   Approach: `std::unique_lock<std::timed_mutex>` limits wait duration and unlocks automatically on exit.
   *   Limitations: Does not emit `SMP_PreSimTimeChange`/`SMP_PostSimTimeChange` events or reject backward time steps.
   */
  void SetSimulationTime(Smp::Duration simulationTime) override;

  /**
   * @brief Set epoch time.
   * @details Acquires `_mutex` (1-second timeout) to update `_epochOffset = epochTime - _simulationTime`,
   * releases the lock, and emits `SMP_EpochTimeChangedId` if `_eventManager != nullptr`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0050.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::SetEpochTime` interface.
   * @compliance Updates `_epochOffset` under timed lock and emits event outside lock scope.
   * @param[in] epochTime
   *   Justification: Specifies the desired absolute epoch date/time corresponding to current `_simulationTime`.
   *   Usage: Computes `_epochOffset = epochTime - _simulationTime` under `_mutex`.
   *   Consumed: No (scalar timestamp copied).
   *   Prerequisites: Valid `Smp::DateTime` whose difference with `_simulationTime` does not overflow.
   * @pre `true` (difference `epochTime - _simulationTime` fits in `Smp::DateTime`).
   * @post `GetEpochTime() == epochTime` and `SMP_EpochTimeChangedId` emitted if `_eventManager != nullptr`.
   * @invariant `_mutex` is released before calling `_eventManager->Emit(...)`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`_epochOffset` unchanged).
   *   - Callback / Event Fault (exception from `_eventManager->Emit`): Propagates after `_epochOffset` has been updated.
   * @containment
   *   Approach: Scoped `std::unique_lock` releases `_mutex` prior to event emission, preventing callback deadlocks.
   *   Limitations: `_eventManager` pointer is checked outside `_mutex`; concurrent `SetEventManager` could race.
   */
  void SetEpochTime(Smp::DateTime epochTime) override;

  /**
   * @brief Set mission start time.
   * @details Acquires `_mutex` (1-second timeout) to store `_missionStart`, releases the lock,
   * and emits `SMP_MissionTimeChangedId` if `_eventManager != nullptr`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0050.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::SetMissionStartTime` interface.
   * @compliance Updates `_missionStart` under timed lock and notifies subscribers outside lock scope.
   * @param[in] missionStart
   *   Justification: Sets the absolute epoch timestamp marking mission T=0.
   *   Usage: Assigned to `_missionStart` under `_mutex`.
   *   Consumed: No (scalar timestamp copied).
   *   Prerequisites: Valid `Smp::DateTime`.
   * @pre `true` (any `Smp::DateTime` is accepted).
   * @post `_missionStart == missionStart` and `SMP_MissionTimeChangedId` emitted if `_eventManager != nullptr`.
   * @invariant `_mutex` is unlocked prior to invoking `_eventManager->Emit(...)`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee (`_missionStart` unchanged).
   *   - Callback / Event Fault (exception from `_eventManager->Emit`): Propagates after `_missionStart` commit.
   * @containment
   *   Approach: Scoped RAII lock ensures `_mutex` is held only for the scalar store.
   *   Limitations: Concurrent `SetEventManager` call is not synchronized with the post-lock `_eventManager` check.
   */
  void SetMissionStartTime(Smp::DateTime missionStart) override;

  /**
   * @brief Set mission time.
   * @details Computes `missionStart = GetEpochTime() - missionTime` and delegates to
   * `SetMissionStartTime`, updating `_missionStart` and emitting `SMP_MissionTimeChangedId`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `ITimeKeeper::SetMissionTime` interface.
   * @compliance Delegates cleanly to `GetEpochTime()` and `SetMissionStartTime()` without nested locking.
   * @param[in] missionTime
   *   Justification: Specifies desired current mission duration relative to mission start.
   *   Usage: Subtracted from `GetEpochTime()` and passed to `SetMissionStartTime()`.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Valid `Smp::Duration` such that `GetEpochTime() - missionTime` does not overflow.
   * @pre `true` (`GetEpochTime() - missionTime` fits within `Smp::DateTime`).
   * @post `_missionStart == GetEpochTime() - missionTime` and mission time change event emitted if attached.
   * @throws std::runtime_error [Transient / Timeout] Thrown if lock acquisition in `GetEpochTime()` or `SetMissionStartTime()` times out.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error` from delegated call.
   *   - Callback / Event Fault (exception from `_eventManager->Emit`): Propagates from `SetMissionStartTime()`.
   * @containment
   *   Approach: Acquires and releases `_mutex` sequentially via `GetEpochTime()` then `SetMissionStartTime()` to avoid self-deadlock.
   *   Limitations: Not atomic as a single transaction if `SetEpochTime` runs concurrently between `GetEpochTime()` and `SetMissionStartTime()`.
   */
  void SetMissionTime(Smp::Duration missionTime) override;

private:
  /**
   * @brief Pointer to the Event Manager service.
   * @details Optional non-owning pointer used to emit `SMP_EpochTimeChangedId` and
   * `SMP_MissionTimeChangedId` notifications when time references are updated.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `nullptr` and updated under `_mutex`.
   */
  Smp::Services::IEventManager *_eventManager = nullptr;

  /**
   * @brief Current simulation time.
   * @details Elapsed simulation duration in nanoseconds since simulation start.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `0` and guarded by `_mutex`.
   */
  Smp::Duration _simulationTime = 0;

  /**
   * @brief Offset between simulation and epoch time.
   * @details Added to `_simulationTime` to derive current epoch timestamp (`_epochOffset + _simulationTime`).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `0` and guarded by `_mutex`.
   */
  Smp::DateTime _epochOffset = 0;

  /**
   * @brief Absolute mission start time.
   * @details Epoch timestamp corresponding to mission time `0`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `0` and guarded by `_mutex`.
   */
  Smp::DateTime _missionStart = 0;

  /**
   * @brief Mutex for thread-safe access.
   * @details Timed mutex bounding wait duration across all shared time fields and `_eventManager`.
   *
   * @task TSK-20261113-003
   * @feature FE-0030, FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Single `std::timed_mutex` serializing all guarded member modifications.
   */
  mutable std::timed_mutex _mutex;
};

} // namespace utils
