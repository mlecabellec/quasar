/**
 * @file Scheduler.hpp
 * @brief Implementation of the SMP Scheduler Service.
 *
 * @details Declares `sched::SchedulerEvent` and `sched::Scheduler`, implementing
 * immediate, simulation-time, mission-time, epoch-time, and placeholder Zulu-time
 * event scheduling, rescheduling, cancellation, and timeline execution under
 * `std::timed_mutex` synchronization.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0090.1, CS-0090.2
 * @compliance Declares `sched::Scheduler` and `sched::SchedulerEvent` in a dedicated
 * header inside namespace `sched` with formal Doxygen contracts and traceability.
 */
#pragma once

// SMP interfaces required by Scheduler.
#include <Smp/IEntryPoint.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>

// Standard atomic and container headers.
#include <atomic>
#include <map>

// Ordered timeline containers and base object header.
#include <mutex>
#include <vector>

#include <core/Object.hpp>

/**
 * @brief Namespace containing SMP scheduling service components.
 * @details Encapsulates `SchedulerEvent` and `Scheduler` within the `sched` module.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Encloses all scheduler declarations within namespace `sched`.
 */
namespace sched {

/**
 * @brief Representation of a scheduled event.
 * @details Value record storing scheduling metadata, repeat counters, time-domain
 * discriminators, and original domain timestamps for a single entry point registered
 * with `Scheduler`.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.32, CS-0010.45, CS-0090.1, CS-0090.2
 * @compliance All scalar and pointer members use explicit default member initializers.
 */
struct SchedulerEvent {
  /**
   * @brief Unique event identifier.
   * @details Monotonic identifier assigned by `Scheduler::CreateEventId()`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Services::EventId id = 0;

  /**
   * @brief Entry point to execute.
   * @details Non-owning pointer to the SMP callback executed when the event fires.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `nullptr` by default; caller owns pointee lifetime.
   */
  const Smp::IEntryPoint *entryPoint = nullptr;

  /**
   * @brief Absolute simulation time or dispatch time.
   * @details Target simulation timestamp in nanoseconds used as the `_timeline` key.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Duration time = 0;

  /**
   * @brief Cycle time for recurring events.
   * @details Interval added to `time` after execution when `repeat != 0`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Duration cycleTime = 0;

  /**
   * @brief Total repeat count.
   * @details `0` indicates one-shot, positive values indicate bounded additional cycles,
   * and negative values (`< 0`) indicate unbounded cyclic repetition until removed.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Int64 repeat = 0;

  /**
   * @brief Current execution count.
   * @details Incremented each time the event executes prior to checking `repeat`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Int64 count = 0;

  /**
   * @brief Sequence ID for stable ordering of events at same time.
   * @details Monotonically assigned insertion order counter.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Int64 sequenceId = 0;

  /**
   * @brief Whether event is based on mission time.
   * @details True when scheduled via `AddMissionTimeEvent`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `false` by default.
   */
  bool isMissionTime = false;

  /**
   * @brief Whether event is based on epoch time.
   * @details True when scheduled via `AddEpochTimeEvent`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `false` by default.
   */
  bool isEpochTime = false;

  /**
   * @brief Whether event is based on Zulu time.
   * @details True for absolute Zulu wall-clock scheduled events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `false` by default.
   */
  bool isZuluTime = false;

  /**
   * @brief Whether event is relative to current Zulu time.
   * @details True for relative Zulu delay scheduled events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `false` by default.
   */
  bool isRelativeZulu = false;

  /**
   * @brief Original epoch time requested.
   * @details Preserves the caller-requested epoch timestamp for epoch-based events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::DateTime epochTime = 0;

  /**
   * @brief Original mission time requested.
   * @details Preserves the caller-requested mission duration for mission-based events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Duration missionTime = 0;

  /**
   * @brief Original Zulu time requested.
   * @details Preserves the caller-requested absolute Zulu wall-clock timestamp.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::DateTime zuluTime = 0;

  /**
   * @brief Original Zulu delay requested.
   * @details Preserves the caller-requested relative Zulu delay duration.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `0` by default.
   */
  Smp::Duration zuluDelay = 0;
};

/**
 * @brief Implementation of the SMP Scheduler Service.
 * @details Contributes to [FE-0070.3.1]. Coordinates immediate and time-ordered simulation
 * event queues (`_immediateEvents`, `_timeline`, `_events`) using `ITimeKeeper` for time
 * conversion/advancement and `_mutex` (`std::timed_mutex`) with 1-second wait bounds.
 * During `ExecuteNextEvent()`, `_mutex` is unlocked around `entryPoint->Execute()` and
 * `_timeKeeper->SetSimulationTime()` calls to allow re-entrant event scheduling.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed SMP `IScheduler` and `IComponent` service boundary.
 * @compliance Uses `std::timed_mutex` for bounded locking, RAII lock guards, explicit iterator
 * types (no `auto`), and null/time validation prior to scheduling.
 * @invariant `_mutex` guards all access to `_events`, `_timeline`, `_immediateEvents`,
 * `_nextEventId`, and `_nextSequenceId`; `_timeKeeper != nullptr` and `_logger != nullptr`.
 */
class Scheduler : public core::Object,
                  public virtual Smp::Services::IScheduler {
public:
  /**
   * @brief Constructor.
   * @details Validates non-null `timeKeeper` and `logger` dependencies, initializes
   * `core::Object` metadata, and sets up empty scheduling queues.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @compliance Enforces mandatory non-null dependency check per [CS-0010.15].
   * @param[in] timeKeeper
   *   Justification: Provides simulation, epoch, and mission time queries and advancement.
   *   Usage: Stored in `_timeKeeper` and checked for `nullptr`.
   *   Consumed: No (non-owning pointer; must outlive this `Scheduler`).
   *   Prerequisites: `timeKeeper != nullptr`.
   * @param[in] logger
   *   Justification: Supplies the SMP logging service required by scheduler construction.
   *   Usage: Stored in `_logger` and checked for `nullptr`.
   *   Consumed: No (non-owning pointer; must outlive this `Scheduler`).
   *   Prerequisites: `logger != nullptr`.
   * @pre `timeKeeper != nullptr && logger != nullptr`
   * @post `_timeKeeper == timeKeeper`, `_logger == logger`, and event queues are empty.
   * @throws std::invalid_argument [Validation / Contract] Thrown if `timeKeeper == nullptr` or `logger == nullptr`.
   * @error
   *   - Validation / Contract (`timeKeeper == nullptr || logger == nullptr`): Throws `std::invalid_argument`; no object created.
   * @containment
   *   Approach: Validates pointers in constructor body before exposing instance.
   *   Limitations: Does not verify whether pointers remain valid throughout simulator shutdown.
   */
  Scheduler(Smp::Services::ITimeKeeper *timeKeeper,
            Smp::Services::ILogger *logger);

  /**
   * @brief Virtual destructor.
   * @details Releases internal event maps, timeline multimaps, and synchronization primitives
   * via RAII without throwing exceptions.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0060.5, CS-0090.1, CS-0090.2
   * @compliance Marked `noexcept` and `override` with defaulted RAII cleanup.
   */
  virtual ~Scheduler() noexcept override = default;

  // IComponent methods

  /**
   * @brief Query the current SMP component lifecycle state.
   * @details Returns the fixed connected state `CSK_Connected`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetState` interface.
   * @compliance Const-qualified accessor returning explicit SMP enumeration state.
   * @post Returns `Smp::ComponentStateKind::CSK_Connected` without mutating `*this`.
   * @return Smp::ComponentStateKind
   *   Justification: Exposes component lifecycle state required by `Smp::IComponent`.
   *   Usage: Caller inspects enum value to verify service readiness.
   *   Consumed: Value returned by copy; internal state unchanged.
   *   Prerequisites: Valid `Scheduler` instance.
   *   Limitations: Always reports `CSK_Connected` regardless of `Connect`/`Disconnect`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or assign locally; no cleanup needed.
   *   Specific Values: Always `Smp::ComponentStateKind::CSK_Connected`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw return path): Always returns `CSK_Connected`.
   * @containment
   *   Approach: Constant return performs no allocation or locking.
   *   Limitations: Virtual ABI omits `noexcept`; lifecycle state machine is stubbed.
   */
  Smp::ComponentStateKind GetState() const override;

  /**
   * @brief Publish component properties to an SMP publication receiver.
   * @details No-op override of `Smp::IComponent::Publish` because `Scheduler` publishes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Publish` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::Publish`.
   * @param[in] receiver
   *   Justification: Required by `Smp::IComponent::Publish` to expose published items.
   *   Usage: Unused; no fields are published.
   *   Consumed: No (non-owning pointer; not dereferenced or retained).
   *   Prerequisites: May be `nullptr` or any `Smp::IPublication *`.
   * @pre `true` (all inputs tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Returns cleanly without mutation.
   * @containment
   *   Approach: Empty body prevents unsafe pointer dereference.
   *   Limitations: Internal event queues are not exposed through SMP publication.
   */
  void Publish(Smp::IPublication *receiver) override;

  /**
   * @brief Configure the component with logger and link registry services.
   * @details No-op override of `Smp::IComponent::Configure`; service pointers are supplied
   * through the constructor.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Configure` interface.
   * @compliance Safe no-op configuration hook.
   * @param[in] logger
   *   Justification: Required by `Smp::IComponent::Configure` to inject a logger.
   *   Usage: Unused; `_logger` is set in constructor.
   *   Consumed: No (not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILogger *`.
   * @param[in] linkRegistry
   *   Justification: Required by `Smp::IComponent::Configure` to inject a link registry.
   *   Usage: Unused in this stub.
   *   Consumed: No (not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILinkRegistry *`.
   * @pre `true` (null pointers tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: Empty body avoids overwriting constructor-validated service pointers.
   *   Limitations: Cannot rebind `_logger` post-construction via `Configure`.
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
   *   Justification: Required by `Smp::IComponent::Connect` to bind to a simulator.
   *   Usage: Unused in this stub.
   *   Consumed: No (not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::ISimulator *`.
   * @pre `true` (null pointer tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: No-op body prevents unguarded pointer dereference.
   *   Limitations: Simulator services are not resolved dynamically from `simulator`.
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
   * @compliance Safe no-op disconnect preserving scheduled state.
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: Empty body avoids mutating scheduled event maps during shutdown.
   *   Limitations: Pending events remain in `_events` and `_timeline`.
   */
  void Disconnect() override;

  /**
   * @brief Retrieve the unique SMP type UUID of this component.
   * @details Returns a reference to a function-local static `Smp::Uuid` `{0, 0, 0, 0, 3}`,
   * avoiding global constants per [CS-0010.30].
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.30, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetUuid` interface.
   * @compliance Uses function-local static UUID and returns a `const` reference.
   * @post Returned reference addresses valid static `Smp::Uuid` `{0, 0, 0, 0, 3}`.
   * @return const Smp::Uuid &
   *   Justification: Exposes component UUID required by `Smp::IComponent::GetUuid`.
   *   Usage: Caller inspects or copies the UUID for service lookup.
   *   Consumed: Borrowed const reference; ownership remains with static storage.
   *   Prerequisites: None.
   *   Limitations: Returns generic placeholder UUID `{0, 0, 0, 0, 3}`.
   *   Lifecycle: Static storage duration; valid for process lifetime.
   *   Modification / Deletion: Immutable (`const &`); caller MUST NOT cast away const or delete.
   *   Specific Values: Reference to `{0, 0, 0, 0, 3}`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw constant reference): Always succeeds.
   * @containment
   *   Approach: Function-local static initialization is thread-safe and allocation-free.
   *   Limitations: Base virtual declaration omits `noexcept`.
   */
  const Smp::Uuid &GetUuid() const override;

  /**
   * @brief Retrieve a published field by full name.
   * @details Stub override returning `nullptr` because `Scheduler` exposes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetField` interface.
   * @compliance Const-qualified stub returning `nullptr`.
   * @param[in] fullName
   *   Justification: Identifies requested field path.
   *   Usage: Ignored; no fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `nullptr` without modifying `*this`.
   * @return Smp::IField *
   *   Justification: Indicates absence of published fields per `Smp::IComponent`.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr `Scheduler` publishes no fields.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids dereferencing `fullName`.
   *   Limitations: Field reflection is unsupported.
   */
  Smp::IField *GetField(Smp::String8 fullName) const override;

  /**
   * @brief Retrieve the collection of all published fields.
   * @details Stub override returning `nullptr` because `Scheduler` publishes no fields.
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
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr Component exposes no field collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Direct `nullptr` return without allocation.
   *   Limitations: Callers must tolerate `nullptr` field collections.
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
   *   Justification: Identifies target field path.
   *   Usage: Ignored; no simple fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs safely ignored).
   * @post Returns default `Smp::AnySimple()` without altering `*this`.
   * @return Smp::AnySimple
   *   Justification: Satisfies `Smp::IComponent::GetSimpleValue` return type.
   *   Usage: Caller checks `.type` before reading value.
   *   Consumed: Returned by value; caller owns the temporary.
   *   Prerequisites: None.
   *   Limitations: Always returns empty `ST_None` value.
   *   Lifecycle: Standalone caller-owned value.
   *   Modification / Deletion: Caller may modify or discard freely; no manual delete.
   *   Specific Values: Default-constructed `Smp::AnySimple`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw stub): Always returns empty `Smp::AnySimple`.
   * @containment
   *   Approach: Value-initialized return avoids accessing `fullName`.
   *   Limitations: Missing field names do not raise exceptions.
   */
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;

  /**
   * @brief Set the simple value of a named field.
   * @details Stub override performing no action because `Scheduler` has no simple fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleValue` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::SetSimpleValue`.
   * @param[in] fullName
   *   Justification: Identifies target field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] value
   *   Justification: Supplies new field value.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and discarded.
   *   Prerequisites: Any `Smp::AnySimple`.
   * @pre `true` (all inputs safely ignored).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body prevents unsafe pointer dereference.
   *   Limitations: Missing fields are silently ignored.
   */
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;

  /**
   * @brief Read an array slice from a named simple array field.
   * @details Stub override performing no action because `Scheduler` has no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleArrayValue` interface.
   * @compliance Const-qualified no-op stub preserving caller buffer.
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
   * @details Stub override performing no action because `Scheduler` has no array fields.
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
   * @details Stub override returning `false` because `Scheduler` manages no child collections.
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
   *   Usage: Caller inspects boolean result.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `Scheduler` does not support child objects.
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
   * @details Stub override returning `false` because `Scheduler` holds no child collections.
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
   *   Usage: Caller inspects boolean result.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `Scheduler` owns no child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Stateless `false` return prevents dangling collection access.
   *   Limitations: Does not distinguish argument errors.
   */
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;

  /**
   * @brief Check whether a named child belongs to a given collection.
   * @details Stub override returning `nullptr` because `Scheduler` holds no child collections.
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
   * @details Stub override returning `nullptr` because `Scheduler` is a leaf service.
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

  // IScheduler methods

  /**
   * @brief Schedule an entry point for immediate one-shot execution.
   * @details Validates `entryPoint`, acquires `_mutex` (1-second timeout), assigns a new
   * `EventId`, records the event in `_events`, and appends its ID to `_immediateEvents`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddImmediateEvent` interface.
   * @compliance Pre-lock null check and bounded RAII `std::timed_mutex` lock.
   * @param[in] entryPoint
   *   Justification: Callback entry point to invoke ahead of timeline events.
   *   Usage: Validated non-null and stored in `SchedulerEvent::entryPoint`.
   *   Consumed: No (non-owning pointer; caller retains pointee ownership).
   *   Prerequisites: `entryPoint != nullptr` and remains valid until executed or removed.
   * @pre `entryPoint != nullptr`
   * @post Returned `EventId` is stored in `_events` and queued at the back of `_immediateEvents`.
   * @invariant `_mutex` protects `_events`, `_immediateEvents`, `_nextEventId`, and `_nextSequenceId`.
   * @return Smp::Services::EventId
   *   Justification: Allows caller to inspect or cancel the immediate event prior to execution.
   *   Usage: Pass to `IsEventScheduled` or `RemoveEvent`.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: Non-null `entryPoint`.
   *   Limitations: Immediate events are one-shot and removed upon execution.
   *   Lifecycle: Valid until executed by `ExecuteNextEvent()` or removed via `RemoveEvent()`.
   *   Modification / Deletion: Caller owns scalar copy; cancel via `RemoveEvent()`.
   *   Specific Values: Positive unique `Smp::Services::EventId` (`>= 1`).
   *   Error Behavior: Throws before returning on null pointer, lock timeout, or allocation failure.
   * @throws std::invalid_argument [Validation / Contract] Thrown before locking if `entryPoint == nullptr`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` acquisition exceeds 1 second.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if map/vector insertion fails.
   * @error
   *   - Validation / Contract (`entryPoint == nullptr`): Throws `std::invalid_argument`; strong guarantee.
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds via RAII lock release; basic guarantee if `_events` inserted before `_immediateEvents` push.
   * @containment
   *   Approach: Null validation precedes locking; timed mutex prevents indefinite stalls.
   *   Limitations: Calls `_timeKeeper->GetSimulationTime()` while holding `_mutex`; if `_immediateEvents.push_back` throws, `_events` retains the orphaned entry.
   */
  Smp::Services::EventId
  AddImmediateEvent(const Smp::IEntryPoint *entryPoint) override;

  /**
   * @brief Schedule an entry point at a target simulation time.
   * @details Validates `entryPoint`, non-negative `simulationTime`, and positive `cycleTime`
   * when `repeat != 0`, then registers and inserts the event into `_events` and `_timeline`
   * under `_mutex` (1-second timeout).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddSimulationTimeEvent` interface.
   * @compliance Enforces null pointer, negative time, and cyclic-interval validation before timed lock.
   * @param[in] entryPoint
   *   Justification: Entry point callback to invoke at `simulationTime`.
   *   Usage: Stored as non-owning pointer in `SchedulerEvent::entryPoint`.
   *   Consumed: No (caller retains ownership).
   *   Prerequisites: `entryPoint != nullptr` and valid until event completion or removal.
   * @param[in] simulationTime
   *   Justification: Target simulation timestamp in nanoseconds.
   *   Usage: Stored in `SchedulerEvent::time` and used as `_timeline` key.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: `simulationTime >= 0`.
   * @param[in] cycleTime
   *   Justification: Period between recurring executions.
   *   Usage: Stored in `SchedulerEvent::cycleTime`; defaults to `0`.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Must be `> 0` whenever `repeat != 0`.
   * @param[in] repeat
   *   Justification: Controls number of repetitions (`0` = once, `< 0` = infinite, `> 0` = count).
   *   Usage: Stored in `SchedulerEvent::repeat`; defaults to `0`.
   *   Consumed: No (scalar integer copied).
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `entryPoint != nullptr && simulationTime >= 0 && (repeat == 0 || cycleTime > 0)`
   * @post Returned `EventId` is stored in `_events` and `_timeline` at key `simulationTime`.
   * @return Smp::Services::EventId
   *   Justification: Unique handle for rescheduling, querying, or canceling the event.
   *   Usage: Passed to `SetEvent*`, `IsEventScheduled`, or `RemoveEvent`.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: Valid parameters satisfying preconditions.
   *   Limitations: Events scheduled prior to current simulation time execute on next step without past-time rejection.
   *   Lifecycle: Valid until final execution or `RemoveEvent()`.
   *   Modification / Deletion: Caller owns scalar handle; cancel via `RemoveEvent()`.
   *   Specific Values: Positive unique `Smp::Services::EventId` (`>= 1`).
   *   Error Behavior: Throws on invalid arguments, invalid time/cycle, or lock timeout.
   * @throws std::invalid_argument [Validation / Contract] Thrown if `entryPoint == nullptr`.
   * @throws core::InvalidEventTime [Validation / Contract] Thrown if `simulationTime < 0`.
   * @throws core::InvalidCycleTime [Validation / Contract] Thrown if `repeat != 0 && cycleTime <= 0`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` lock times out after 1 second.
   * @error
   *   - Validation / Contract (null pointer, negative time, invalid cycle): Throws before locking; strong guarantee.
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds via RAII lock release.
   * @containment
   *   Approach: Pre-lock argument validation prevents partial mutation; timed lock bounds wait.
   *   Limitations: If `_timeline.insert` fails after `_events[evt.id] = evt`, `_events` retains an unscheduled record.
   */
  Smp::Services::EventId AddSimulationTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration simulationTime,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;

  /**
   * @brief Schedule an entry point at a target mission time.
   * @details Validates `entryPoint`, ensures `missionTime >= _timeKeeper->GetMissionTime()`,
   * validates `cycleTime` for cyclic events, converts `missionTime` into simulation time,
   * and inserts the event under `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddMissionTimeEvent` interface.
   * @compliance Enforces null, past-time, and cycle-time checks prior to timed mutex acquisition.
   * @param[in] entryPoint
   *   Justification: Callback entry point to execute at `missionTime`.
   *   Usage: Stored as non-owning pointer in `SchedulerEvent::entryPoint`.
   *   Consumed: No (caller retains ownership).
   *   Prerequisites: `entryPoint != nullptr`.
   * @param[in] missionTime
   *   Justification: Target elapsed mission duration in nanoseconds.
   *   Usage: Stored in `SchedulerEvent::missionTime` and mapped to simulation time.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: `missionTime >= _timeKeeper->GetMissionTime()`.
   * @param[in] cycleTime
   *   Justification: Period between recurring executions.
   *   Usage: Stored in `SchedulerEvent::cycleTime`; defaults to `0`.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Must be `> 0` when `repeat != 0`.
   * @param[in] repeat
   *   Justification: Repetition policy (`0` = once, `< 0` = infinite, `> 0` = count).
   *   Usage: Stored in `SchedulerEvent::repeat`; defaults to `0`.
   *   Consumed: No (scalar integer copied).
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `entryPoint != nullptr && missionTime >= _timeKeeper->GetMissionTime() && (repeat == 0 || cycleTime > 0)`
   * @post Event registered in `_events` and `_timeline` with `isMissionTime == true`.
   * @return Smp::Services::EventId
   *   Justification: Unique handle for querying or canceling the scheduled mission event.
   *   Usage: Pass to `IsEventScheduled` or `RemoveEvent`.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: Valid non-past `missionTime` and non-null `entryPoint`.
   *   Limitations: Mission-time offset changes are not automatically re-mapped after scheduling unless rescheduled.
   *   Lifecycle: Valid until final execution or `RemoveEvent()`.
   *   Modification / Deletion: Caller owns scalar copy; cancel via `RemoveEvent()`.
   *   Specific Values: Positive unique `Smp::Services::EventId` (`>= 1`).
   *   Error Behavior: Throws on invalid pointer, past mission time, invalid cycle, or lock timeout.
   * @throws std::invalid_argument [Validation / Contract] Thrown if `entryPoint == nullptr`.
   * @throws core::InvalidEventTime [Validation / Contract] Thrown if `missionTime < _timeKeeper->GetMissionTime()`.
   * @throws core::InvalidCycleTime [Validation / Contract] Thrown if `repeat != 0 && cycleTime <= 0`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` or `_timeKeeper` lock times out.
   * @error
   *   - Validation / Contract (null, past mission time, bad cycle): Throws prior to locking `_mutex`; strong guarantee.
   *   - Transient / Timeout (lock timeout): Throws `std::runtime_error`; strong guarantee.
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds cleanly via RAII lock release.
   * @containment
   *   Approach: Pre-lock checks prevent partial state updates; RAII timed lock prevents deadlock.
   *   Limitations: Calls `_timeKeeper` time getters while holding `_mutex`; assumes `_timeKeeper->GetMissionTime()` does not self-deadlock.
   */
  Smp::Services::EventId AddMissionTimeEvent(const Smp::IEntryPoint *entryPoint,
                                             Smp::Duration missionTime,
                                             Smp::Duration cycleTime = 0,
                                             Smp::Int64 repeat = 0) override;

  /**
   * @brief Schedule an entry point at a target epoch time.
   * @details Validates `entryPoint`, checks `epochTime >= _timeKeeper->GetEpochTime()`,
   * validates `cycleTime` when `repeat != 0`, translates `epochTime` to simulation time,
   * and registers the event under `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddEpochTimeEvent` interface.
   * @compliance Enforces null, past-epoch, and cycle-time checks prior to timed mutex acquisition.
   * @param[in] entryPoint
   *   Justification: Callback entry point to execute at `epochTime`.
   *   Usage: Stored as non-owning pointer in `SchedulerEvent::entryPoint`.
   *   Consumed: No (caller retains ownership).
   *   Prerequisites: `entryPoint != nullptr`.
   * @param[in] epochTime
   *   Justification: Target absolute epoch timestamp.
   *   Usage: Stored in `SchedulerEvent::epochTime` and converted to simulation time.
   *   Consumed: No (scalar timestamp copied).
   *   Prerequisites: `epochTime >= _timeKeeper->GetEpochTime()`.
   * @param[in] cycleTime
   *   Justification: Period between recurring executions.
   *   Usage: Stored in `SchedulerEvent::cycleTime`; defaults to `0`.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Must be `> 0` when `repeat != 0`.
   * @param[in] repeat
   *   Justification: Repetition count (`0` = once, `< 0` = infinite, `> 0` = count).
   *   Usage: Stored in `SchedulerEvent::repeat`; defaults to `0`.
   *   Consumed: No (scalar integer copied).
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `entryPoint != nullptr && epochTime >= _timeKeeper->GetEpochTime() && (repeat == 0 || cycleTime > 0)`
   * @post Event registered in `_events` and `_timeline` with `isEpochTime == true`.
   * @return Smp::Services::EventId
   *   Justification: Unique handle for querying or removing the epoch event.
   *   Usage: Pass to `IsEventScheduled` or `RemoveEvent`.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: Valid non-past `epochTime` and non-null `entryPoint`.
   *   Limitations: Dynamic `SetEpochTime` adjustments do not automatically re-index existing `_timeline` entries.
   *   Lifecycle: Valid until final execution or `RemoveEvent()`.
   *   Modification / Deletion: Caller owns scalar copy; cancel via `RemoveEvent()`.
   *   Specific Values: Positive unique `Smp::Services::EventId` (`>= 1`).
   *   Error Behavior: Throws on null pointer, past epoch time, invalid cycle, or lock timeout.
   * @throws std::invalid_argument [Validation / Contract] Thrown if `entryPoint == nullptr`.
   * @throws core::InvalidEventTime [Validation / Contract] Thrown if `epochTime < _timeKeeper->GetEpochTime()`.
   * @throws core::InvalidCycleTime [Validation / Contract] Thrown if `repeat != 0 && cycleTime <= 0`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` or `_timeKeeper` lock times out.
   * @error
   *   - Validation / Contract (null, past epoch time, bad cycle): Throws prior to locking `_mutex`; strong guarantee.
   *   - Transient / Timeout (lock timeout): Throws `std::runtime_error`; strong guarantee.
   *   - Resource / Allocation (`std::bad_alloc`): Unwinds cleanly via RAII lock release.
   * @containment
   *   Approach: Pre-lock checks prevent partial state updates; RAII timed lock bounds wait time.
   *   Limitations: Holds `_mutex` while calling `_timeKeeper->GetEpochTime()` and `GetSimulationTime()`.
   */
  Smp::Services::EventId AddEpochTimeEvent(const Smp::IEntryPoint *entryPoint,
                                           Smp::DateTime epochTime,
                                           Smp::Duration cycleTime = 0,
                                           Smp::Int64 repeat = 0) override;

  /**
   * @brief Schedule an entry point at an absolute Zulu wall-clock time.
   * @details Placeholder stub returning `-1`; absolute Zulu wall-clock scheduling is not
   * yet implemented in this scheduler revision.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddZuluTimeEvent` interface.
   * @compliance Returns explicit invalid sentinel `-1` without mutating scheduler queues.
   * @param[in] entryPoint
   *   Justification: Required by `Smp::Services::IScheduler` to specify the callback.
   *   Usage: Ignored by placeholder stub.
   *   Consumed: No (not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or valid `const Smp::IEntryPoint *`.
   * @param[in] zuluTime
   *   Justification: Required by `Smp::Services::IScheduler` for target wall-clock timestamp.
   *   Usage: Ignored by placeholder stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::DateTime`.
   * @param[in] cycleTime
   *   Justification: Required by `Smp::Services::IScheduler` for cyclic interval.
   *   Usage: Ignored by placeholder stub; defaults to `0`.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Duration`.
   * @param[in] repeat
   *   Justification: Required by `Smp::Services::IScheduler` for repeat count.
   *   Usage: Ignored by placeholder stub; defaults to `0`.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `true` (stub tolerates all inputs safely).
   * @post Returns `-1` and `*this` remains unchanged.
   * @return Smp::Services::EventId
   *   Justification: Signals unsupported Zulu scheduling via sentinel `-1`.
   *   Usage: Caller must check for `-1` before treating return value as an active event ID.
   *   Consumed: Scalar sentinel returned by copy.
   *   Prerequisites: None.
   *   Limitations: Zulu-time events are not scheduled or executed.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard scalar; do not pass `-1` to `RemoveEvent`.
   *   Specific Values: Always `-1` (unimplemented placeholder).
   *   Error Behavior: Never throws; returns `-1`.
   * @retval -1 Zulu-time event scheduling is not implemented.
   * @error
   *   - Unsupported Feature (placeholder stub): Returns `-1` cleanly; no-throw.
   * @containment
   *   Approach: Stateless `-1` sentinel return avoids registering unexecutable Zulu timers.
   *   Limitations: Callers expecting valid positive `EventId` receive `-1`.
   */
  Smp::Services::EventId AddZuluTimeEvent(const Smp::IEntryPoint *entryPoint,
                                          Smp::DateTime zuluTime,
                                          Smp::Duration cycleTime = 0,
                                          Smp::Int64 repeat = 0) override;

  /**
   * @brief Schedule an entry point after a relative Zulu wall-clock delay.
   * @details Placeholder stub returning `-1`; relative Zulu wall-clock scheduling is not
   * yet implemented in this scheduler revision.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::AddRelativeZuluTimeEvent` interface.
   * @compliance Returns explicit invalid sentinel `-1` without modifying internal maps.
   * @param[in] entryPoint
   *   Justification: Required by `Smp::Services::IScheduler` to specify the callback.
   *   Usage: Ignored by placeholder stub.
   *   Consumed: No (not dereferenced or stored).
   *   Prerequisites: May be `nullptr` or valid `const Smp::IEntryPoint *`.
   * @param[in] deltaZuluTime
   *   Justification: Required by `Smp::Services::IScheduler` for relative wall-clock delay.
   *   Usage: Ignored by placeholder stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Duration`.
   * @param[in] cycleTime
   *   Justification: Required by `Smp::Services::IScheduler` for cyclic interval.
   *   Usage: Ignored by placeholder stub; defaults to `0`.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Duration`.
   * @param[in] repeat
   *   Justification: Required by `Smp::Services::IScheduler` for repeat count.
   *   Usage: Ignored by placeholder stub; defaults to `0`.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `true` (stub tolerates all inputs safely).
   * @post Returns `-1` and `*this` remains unchanged.
   * @return Smp::Services::EventId
   *   Justification: Signals unsupported relative Zulu scheduling via sentinel `-1`.
   *   Usage: Caller must check for `-1` before using the returned ID.
   *   Consumed: Scalar sentinel returned by copy.
   *   Prerequisites: None.
   *   Limitations: Relative Zulu-time events are not scheduled or executed.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard scalar; do not pass `-1` to `RemoveEvent`.
   *   Specific Values: Always `-1` (unimplemented placeholder).
   *   Error Behavior: Never throws; returns `-1`.
   * @retval -1 Relative Zulu-time event scheduling is not implemented.
   * @error
   *   - Unsupported Feature (placeholder stub): Returns `-1` cleanly; no-throw.
   * @containment
   *   Approach: Stateless `-1` sentinel return avoids registering unexecutable timers.
   *   Limitations: Callers expecting valid positive `EventId` receive `-1`.
   */
  Smp::Services::EventId AddRelativeZuluTimeEvent(
      const Smp::IEntryPoint *entryPoint, Smp::Duration deltaZuluTime,
      Smp::Duration cycleTime = 0, Smp::Int64 repeat = 0) override;

  /**
   * @brief Update the simulation execution time of an existing simulation-time event.
   * @details Acquires `_mutex` (1-second timeout), verifies `event` exists and is not a
   * mission/epoch/Zulu event, removes the event if `simulationTime < 0`, or otherwise
   * updates its time and re-indexes it in `_timeline`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventSimulationTime` interface.
   * @compliance Enforces timed lock, event-domain checks, and clean timeline re-insertion.
   * @param[in] event
   *   Justification: Identifies the scheduled simulation-time event to update or cancel.
   *   Usage: Looked up in `_events` under `_mutex`.
   *   Consumed: No (scalar ID copied).
   *   Prerequisites: Active `EventId` for a simulation-time event currently in `_events`.
   * @param[in] simulationTime
   *   Justification: New target simulation timestamp (or `< 0` to remove the event).
   *   Usage: Cancels event if `< 0`, otherwise updates `SchedulerEvent::time` and re-schedules.
   *   Consumed: No (scalar duration copied).
   *   Prerequisites: Any `Smp::Duration`.
   * @pre `event` exists in `_events` and is a simulation-time event (`!isMissionTime && !isEpochTime && !isZuluTime`).
   * @post If `simulationTime < 0`, `event` is removed; otherwise `event` is scheduled at `simulationTime`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if initial or post-removal `_mutex` acquisition times out.
   * @throws core::InvalidEventId [Validation / Contract] Thrown if `event` is missing or belongs to another time domain.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee on initial lock.
   *   - Validation / Contract (unknown or wrong-domain `event`): Throws `core::InvalidEventId`; strong guarantee.
   * @containment
   *   Approach: Releases `lock` before calling `RemoveEvent(event)` when `simulationTime < 0` to avoid non-recursive self-deadlock, then re-acquires with 1s timeout.
   *   Limitations: Concurrent modification could occur during the brief unlock window when `simulationTime < 0`.
   */
  void SetEventSimulationTime(Smp::Services::EventId event,
                              Smp::Duration simulationTime) override;

  /**
   * @brief Update the mission execution time of a scheduled event.
   * @details Stub override; mission-time rescheduling is not yet implemented.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventMissionTime` interface.
   * @compliance Safe no-op stub matching `Smp::Services::IScheduler` signature.
   * @param[in] event
   *   Justification: Required by `IScheduler` to identify the event.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @param[in] missionTime
   *   Justification: Required by `IScheduler` for the new mission time.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Duration`.
   * @pre `true` (stub ignores inputs safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body prevents corrupting timeline entries.
   *   Limitations: Mission-time rescheduling is unsupported.
   */
  void SetEventMissionTime(Smp::Services::EventId event,
                           Smp::Duration missionTime) override;

  /**
   * @brief Update the epoch execution time of a scheduled event.
   * @details Stub override; epoch-time rescheduling is not yet implemented.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventEpochTime` interface.
   * @compliance Safe no-op stub matching `Smp::Services::IScheduler` signature.
   * @param[in] event
   *   Justification: Required by `IScheduler` to identify the event.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @param[in] epochTime
   *   Justification: Required by `IScheduler` for the new epoch time.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::DateTime`.
   * @pre `true` (stub ignores inputs safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body prevents corrupting timeline entries.
   *   Limitations: Epoch-time rescheduling is unsupported.
   */
  void SetEventEpochTime(Smp::Services::EventId event,
                         Smp::DateTime epochTime) override;

  /**
   * @brief Update the Zulu execution time of a scheduled event.
   * @details Stub override; Zulu-time rescheduling is not yet implemented.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventZuluTime` interface.
   * @compliance Safe no-op stub matching `Smp::Services::IScheduler` signature.
   * @param[in] event
   *   Justification: Required by `IScheduler` to identify the event.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @param[in] zuluTime
   *   Justification: Required by `IScheduler` for the new Zulu time.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::DateTime`.
   * @pre `true` (stub ignores inputs safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body prevents corrupting timeline entries.
   *   Limitations: Zulu-time rescheduling is unsupported.
   */
  void SetEventZuluTime(Smp::Services::EventId event,
                        Smp::DateTime zuluTime) override;

  /**
   * @brief Update the cycle time of a scheduled recurring event.
   * @details Stub override; dynamic cycle-time mutation is not yet implemented.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventCycleTime` interface.
   * @compliance Safe no-op stub matching `Smp::Services::IScheduler` signature.
   * @param[in] event
   *   Justification: Required by `IScheduler` to identify the event.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @param[in] cycleTime
   *   Justification: Required by `IScheduler` for the new cycle duration.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Duration`.
   * @pre `true` (stub ignores inputs safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body avoids unvalidated cycle updates.
   *   Limitations: Cycle time cannot be changed after scheduling.
   */
  void SetEventCycleTime(Smp::Services::EventId event,
                         Smp::Duration cycleTime) override;

  /**
   * @brief Update the repeat count of a scheduled event.
   * @details Stub override; dynamic repeat-count mutation is not yet implemented.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::SetEventRepeat` interface.
   * @compliance Safe no-op stub matching `Smp::Services::IScheduler` signature.
   * @param[in] event
   *   Justification: Required by `IScheduler` to identify the event.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @param[in] repeat
   *   Justification: Required by `IScheduler` for the new repeat limit.
   *   Usage: Unused in this stub.
   *   Consumed: Scalar copied and unused.
   *   Prerequisites: Any `Smp::Int64`.
   * @pre `true` (stub ignores inputs safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): No mutation or exception occurs.
   * @containment
   *   Approach: Empty body avoids unvalidated repeat-count updates.
   *   Limitations: Repeat count cannot be changed after scheduling.
   */
  void SetEventRepeat(Smp::Services::EventId event, Smp::Int64 repeat) override;

  /**
   * @brief Remove a scheduled event from the scheduler.
   * @details Acquires `_mutex` (1-second timeout), locates `event` in `_events`,
   * removes all matching timeline entries and immediate queue entries, and erases
   * the record from `_events`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::RemoveEvent` interface.
   * @compliance Enforces RAII timed lock, validates event existence, and uses explicit iterators.
   * @param[in] event
   *   Justification: Identifies the scheduled event to cancel and purge.
   *   Usage: Searched in `_events`, `_timeline`, and `_immediateEvents` under `_mutex`.
   *   Consumed: No (scalar ID copied).
   *   Prerequisites: `event` must be currently present in `_events`.
   * @pre `event` is currently registered in `_events`.
   * @post `event` is absent from `_events`, `_timeline`, and `_immediateEvents`.
   * @invariant No dangling `EventId` remains in `_timeline` or `_immediateEvents` after return.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @throws core::InvalidEventId [Validation / Contract] Thrown if `event` is not found in `_events`.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   *   - Validation / Contract (unknown `event`): Throws `core::InvalidEventId`; strong guarantee.
   * @containment
   *   Approach: Validates membership in `_events` under timed lock before erasing from any queue.
   *   Limitations: Cannot cancel an event that is currently mid-execution unless still in `_events`.
   */
  void RemoveEvent(Smp::Services::EventId event) override;

  /**
   * @brief Check whether a given event ID is currently scheduled.
   * @details Acquires `_mutex` (1-second timeout) and checks whether `event` exists in `_events`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::IsEventScheduled` interface.
   * @compliance Const-qualified lookup protected by `std::timed_mutex`.
   * @param[in] event
   *   Justification: Event identifier whose active status is queried.
   *   Usage: Searched in `_events` under `_mutex`.
   *   Consumed: No (scalar ID copied).
   *   Prerequisites: Any `Smp::Services::EventId`.
   * @pre `true` (invalid/unknown IDs safely return `false`).
   * @post Returns `true` iff `event` exists in `_events`; `*this` unchanged.
   * @return Smp::Bool
   *   Justification: Reports whether `event` remains active in the scheduler.
   *   Usage: Caller checks boolean result before modifying or waiting on `event`.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Status reflects snapshot at lock release and may change immediately on concurrent `ExecuteNextEvent`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: `true` if `event` is in `_events`; `false` otherwise.
   *   Error Behavior: Throws `std::runtime_error` if timed lock acquisition fails.
   * @retval true `event` is present in `_events`.
   * @retval false `event` is not present in `_events`.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   * @containment
   *   Approach: `std::unique_lock<std::timed_mutex>` bounds wait time and guarantees RAII unlock.
   *   Limitations: Contention exceeding 1 second raises an exception instead of returning a fallback.
   */
  Smp::Bool IsEventScheduled(Smp::Services::EventId event) const override;

  /**
   * @brief Retrieve the identifier of the event currently executing.
   * @details Returns `_currentEventId` (`-1` when no event callback is actively executing).
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::GetCurrentEventId` interface.
   * @compliance Const-qualified accessor returning `_currentEventId`.
   * @post Returns `_currentEventId` without modifying `*this`.
   * @return Smp::Services::EventId
   *   Justification: Allows callbacks or observers to identify the currently running scheduled event.
   *   Usage: Inspect value; `-1` denotes no active event execution.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: None.
   *   Limitations: Reads `_currentEventId` without locking `_mutex`; intended for simulation-thread callback inspection.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Active `EventId` (`>= 1`) during execution, or `-1` when idle.
   *   Error Behavior: Never throws.
   * @retval -1 No event is currently executing.
   * @error
   *   - None (no-throw accessor): Returns `_currentEventId` directly.
   * @containment
   *   Approach: Lock-free read avoids deadlocking if called from inside an executing `IEntryPoint`.
   *   Limitations: Not synchronized with `_mutex` if queried concurrently from non-simulation threads.
   */
  Smp::Services::EventId GetCurrentEventId() const override;

  /**
   * @brief Retrieve the simulation time of the next scheduled timeline event.
   * @details Acquires `_mutex` (1-second timeout) and returns `_timeline.begin()->first`
   * if `_timeline` is non-empty, or `std::numeric_limits<Smp::Duration>::max()` otherwise.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IScheduler::GetNextScheduledEventTime` interface.
   * @compliance Const-qualified query protected by `std::timed_mutex` with explicit empty-timeline sentinel.
   * @post Returns earliest `_timeline` key or `Duration::max()` without modifying `*this`.
   * @return Smp::Duration
   *   Justification: Enables simulation loops to determine the next time advance target.
   *   Usage: Compare against `Duration::max()` before advancing simulation time.
   *   Consumed: Scalar duration returned by copy.
   *   Prerequisites: None.
   *   Limitations: Ignores `_immediateEvents`; check `HasEvents()` or execute immediate events first.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Earliest scheduled `Smp::Duration`, or `std::numeric_limits<Smp::Duration>::max()` when empty.
   *   Error Behavior: Throws `std::runtime_error` on lock timeout.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   * @containment
   *   Approach: Bounded RAII lock prevents deadlock and checks `!_timeline.empty()` before dereferencing `begin()`.
   *   Limitations: Lock contention > 1s throws rather than returning a sentinel.
   */
  Smp::Duration GetNextScheduledEventTime() const override;

  /**
   * @brief Execute the next event in the timeline.
   * @details Processes `_immediateEvents` first (one-shot), or else pops the earliest
   * event from `_timeline`, advances `_timeKeeper` simulation time if needed, invokes
   * `ep->Execute()` with `_mutex` unlocked, and either reschedules cyclic events or
   * erases finished events from `_events`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0050.2, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @compliance Uses bounded timed mutex, releases lock around external callbacks and `_timeKeeper`
   * time updates, and enforces explicit iterator types.
   * @pre `_timeKeeper != nullptr`
   * @post One pending event (if any) has executed and been erased or rescheduled; `_currentEventId == -1` on normal exit.
   * @invariant `_mutex` is unlocked during `ep->Execute()` and `_timeKeeper->SetSimulationTime()`.
   * @return Smp::Duration
   *   Justification: Reports the simulation timestamp at which an event executed, or `-1` when idle.
   *   Usage: Simulation loop inspects `>= 0` vs `-1` to step or terminate.
   *   Consumed: Scalar duration returned by copy.
   *   Prerequisites: Valid `Scheduler` constructed with non-null `_timeKeeper`.
   *   Limitations: Assumes `_timeline` head event is not concurrently erased during the brief unlock for time advancement.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Simulation execution timestamp on dispatch, or `-1` if no event was executed.
   *   Error Behavior: Throws `std::runtime_error` on lock/re-lock timeout or propagates callback exceptions.
   * @retval -1 No immediate or timeline events were pending.
   * @throws std::runtime_error [Transient / Timeout] Thrown if initial lock or post-callback re-lock times out after 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` lock or re-lock > 1s): Throws `std::runtime_error`.
   *   - Callback / TimeKeeper Fault (exception in `ep->Execute()` or `_timeKeeper`): Propagates while `_mutex` is unlocked; `_currentEventId` may remain set if callback throws.
   * @containment
   *   Approach: Releases `_mutex` around external callbacks so entry points can safely call `Add*Event`/`RemoveEvent`, and uses 1s timed re-lock to bound deadlock recovery.
   *   Limitations: If `ep->Execute()` throws, `_currentEventId` is not reset via RAII guard and cyclic rescheduling is skipped; `_events[id]` is recreated if callback removed itself during timeline execution.
   */
  Smp::Duration ExecuteNextEvent();

  /**
   * @brief Check if there are any events scheduled.
   * @details Acquires `_mutex` (1-second timeout) and returns `!_timeline.empty() || !_immediateEvents.empty()`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @compliance Const-qualified query protected by `std::timed_mutex`.
   * @post Returns `true` iff at least one immediate or timeline event is queued; `*this` unchanged.
   * @return bool
   *   Justification: Drives simulation stepping loops to determine if work remains.
   *   Usage: Loop condition checks `HasEvents()` before calling `ExecuteNextEvent()`.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Reflects state at lock release; concurrent threads could add/remove events afterwards.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: `true` if `_timeline` or `_immediateEvents` is non-empty; `false` otherwise.
   *   Error Behavior: Throws `std::runtime_error` if timed lock acquisition fails.
   * @retval true At least one immediate or timeline event is queued.
   * @retval false Both `_timeline` and `_immediateEvents` are empty.
   * @throws std::runtime_error [Transient / Timeout] Thrown if `_mutex` cannot be acquired within 1 second.
   * @error
   *   - Transient / Timeout (`_mutex` busy > 1s): Throws `std::runtime_error`; strong guarantee.
   * @containment
   *   Approach: `std::unique_lock<std::timed_mutex>` bounds wait time and unlocks automatically.
   *   Limitations: Lock contention > 1s throws rather than returning `false`.
   */
  bool HasEvents() const;

private:
  /**
   * @brief Pointer to the TimeKeeper service.
   * @details Non-owning service dependency used to query and advance simulation, epoch,
   * and mission times. Validated non-null in constructor.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `nullptr` and validated non-null in `Scheduler()`.
   */
  Smp::Services::ITimeKeeper *_timeKeeper = nullptr;

  /**
   * @brief Pointer to the Logger service.
   * @details Non-owning service dependency retained for scheduler diagnostics. Validated
   * non-null in constructor.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Initialized to `nullptr` and validated non-null in `Scheduler()`.
   */
  Smp::Services::ILogger *_logger = nullptr;

  /**
   * @brief Mutex for thread-safe access.
   * @details Timed mutex serializing all accesses to `_events`, `_timeline`,
   * `_immediateEvents`, `_nextEventId`, and `_nextSequenceId`.
   *
   * @task TSK-20261113-003
   * @feature FE-0030, FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Single `std::timed_mutex` protecting all mutable scheduler queues.
   */
  mutable std::timed_mutex _mutex;

  /**
   * @brief ID counter for new events.
   * @details Starts at `1` and increments per newly registered event under `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `1` and incremented only under `_mutex`.
   */
  Smp::Services::EventId _nextEventId = 1;

  /**
   * @brief Sequence ID counter for stable ordering.
   * @details Monotonically incremented on each event insertion under `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `0` and updated under `_mutex`.
   */
  Smp::Int64 _nextSequenceId = 0;

  /**
   * @brief Collection of all events by ID.
   * @details Primary owner map storing `SchedulerEvent` records keyed by `EventId`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Accessed exclusively while holding `_mutex`.
   */
  std::map<Smp::Services::EventId, SchedulerEvent> _events;

  /**
   * @brief Ordered execution timeline. Map key is Simulation Time.
   * @details Multimap indexing active non-immediate events by target simulation `Duration`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Accessed exclusively while holding `_mutex`.
   */
  std::multimap<Smp::Duration, Smp::Services::EventId> _timeline;

  /**
   * @brief List of immediate events to execute.
   * @details FIFO queue of `EventId` values dispatched ahead of `_timeline` events.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Accessed exclusively while holding `_mutex`.
   */
  std::vector<Smp::Services::EventId> _immediateEvents;

  /**
   * @brief ID of the event currently being executed.
   * @details Holds the active `EventId` during `ExecuteNextEvent()` callback execution,
   * or `-1` when no event is executing.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.32, CS-0090.1, CS-0090.2
   * @compliance Explicitly initialized to `-1`.
   */
  Smp::Services::EventId _currentEventId = -1;

  /**
   * @brief Create a new unique event ID.
   * @details Post-increments `_nextEventId` and returns the allocated identifier.
   * Caller MUST hold `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @compliance Private helper invoked solely while `_mutex` is locked.
   * @pre `_mutex` is held by calling thread.
   * @post `_nextEventId` is incremented by `1` and the prior value is returned.
   * @return Smp::Services::EventId
   *   Justification: Generates unique monotonic `EventId` keys for `_events`.
   *   Usage: Assigned to `SchedulerEvent::id` during event creation.
   *   Consumed: Scalar `EventId` returned by copy.
   *   Prerequisites: Caller holds `_mutex`.
   *   Limitations: Does not check signed 64-bit wrap-around after `2^63 - 1` events.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: Monotonically increasing integers starting at `1`.
   *   Error Behavior: Never throws.
   * @error
   *   - None (no-throw private helper): Post-increments `_nextEventId` directly.
   * @containment
   *   Approach: Private visibility restricts calls to locked public scheduling methods.
   *   Limitations: Assumes total scheduled events over process lifetime remain `< INT64_MAX`.
   */
  Smp::Services::EventId CreateEventId();

  /**
   * @brief Add an event to the execution timeline.
   * @details Inserts `{evt.time, evt.id}` into `_timeline` when `evt` is neither a
   * Zulu-time nor relative-Zulu-time event. Caller MUST hold `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @compliance Private helper operating strictly under caller-held `_mutex`.
   * @param[in] evt
   *   Justification: Supplies target `time`, `id`, and Zulu flags for timeline insertion.
   *   Usage: Read via const reference; inserts `{evt.time, evt.id}` into `_timeline` if non-Zulu.
   *   Consumed: No (read-only const reference).
   *   Prerequisites: Caller holds `_mutex`; `evt.id` already registered in `_events`.
   * @pre `_mutex` is held by calling thread and `evt.id` is valid.
   * @post If `!evt.isZuluTime && !evt.isRelativeZulu`, `{evt.time, evt.id}` is inserted in `_timeline`.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if `_timeline.insert` allocation fails.
   * @error
   *   - Resource / Allocation (`std::bad_alloc`): `_timeline` remains unchanged (strong guarantee per `std::multimap::insert`).
   * @containment
   *   Approach: `std::multimap::insert` provides strong exception safety; Zulu guard prevents invalid timeline keys.
   *   Limitations: Assumes caller already holds `_mutex`.
   */
  void ScheduleEvent(const SchedulerEvent &evt);

  /**
   * @brief Remove an event from the execution timeline.
   * @details Scans `_timeline` linearly and erases every node whose `.second == id`.
   * Caller MUST hold `_mutex`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.34, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @compliance Uses explicit multimap iterator types (no `auto`) and safe `erase(it)` iteration under `_mutex`.
   * @param[in] id
   *   Justification: Identifies the `EventId` to purge from `_timeline`.
   *   Usage: Compared against `it->second` across `_timeline`.
   *   Consumed: No (scalar ID copied).
   *   Prerequisites: Caller holds `_mutex`.
   * @pre `_mutex` is held by calling thread.
   * @post No entry in `_timeline` has `.second == id`.
   * @error
   *   - None (no-throw erasure): `std::multimap::erase(iterator)` does not throw.
   * @containment
   *   Approach: Iterator-safe post-erase assignment (`it = _timeline.erase(it)`) avoids iterator invalidation.
   *   Limitations: Linear O(N) scan over `_timeline` bounded by active scheduled event count.
   */
  void RemoveFromTimeline(Smp::Services::EventId id);
};

} // namespace sched
