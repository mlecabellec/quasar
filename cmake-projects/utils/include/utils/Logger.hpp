/**
 * @file Logger.hpp
 * @brief Implementation of the SMP Logger Service.
 *
 * @details Declares `utils::Logger`, providing thread-safe message formatting and
 * logging capabilities to standard output for the SMP simulation framework.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0090.1, CS-0090.2
 * @compliance Declares `utils::Logger` in a dedicated self-contained header
 * inside namespace `utils` with formal Doxygen traceability and contracts.
 *
 * @note This utility class implements SMP services and is thread-safe.
 *       It does not directly contribute to the features described in FE-0020 (NamedObject, tree/graph structures).
 *       The thread safety implemented here is for the Logger's own operations and not related to FE-0020 requirements.
 *       For example, FE-0020.5 requires thread-safe operations on named objects, which are not implemented in this file.
 *
 * @par Contribution to FE-0080 (Catalogue to C++ Mapping & Type Mapping):
 * - This class is a C++ implementation of the SMP ILogger service, aligning with [FE-0080.6.13] (Service types shall be mapped to C++ classes).
 * - Declared within the `utils` namespace, adhering to [FE-0080.5.3] (Elements shall be declared within the exact same namespace as in the Catalogue).
 * - Defined in a dedicated header file (`Logger.hpp`), supporting [FE-0080.5.4] (Each type shall be declared in a dedicated header file), [FE-0080.5.5] (Header files shall allow multiple inclusion), and [FE-0080.5.6] (Header files shall avoid circular dependencies).
 * - Utilizes C++ standard features like `std::timed_mutex`, aligning with [FE-0080.4.1] (C++ standard version at least C++11).
 * - Member variables like `_mutex` represent mapped fields or simple value elements, according to [FE-0080.5.9] and [FE-0080.5.13].
 * - Member methods like `QueryLogMessageKind`, `Log` represent mapped operations, adhering to [FE-0080.5.20] (Operation elements shall be mapped to C++ member methods).
 * - The `GetUuid()` method returns an `Smp::Uuid`, contributing to [FE-0080.6.1] (UUID variable declaration).
 * - Visibility is managed using C++ access specifiers (`public`, `private`), mapping to [FE-0080.5.7] (Visibility kind attributes shall be mapped to ISO/ANSI C++ member access specifiers).
 * - **Contribution to FE-0090 (Datacodec):** This class does not directly contribute to FE-0090. Its purpose is message logging and it is not involved in binary data serialization, deserialization, schema definitions, or codec implementations.
 *
 * @par Contribution to FE-0030:
 * - This class implements the SMP ILogger service and does not directly implement the `Number`, `String`, `Buffer`, or `BitBuffer` types or their named variants as described in FE-0030.1-7.
 * - It adheres to FE-0030.8 by utilizing `std::timed_mutex` for thread-safe operations.
 * - It adheres to FE-0030.9 by exhibiting `const` correctness in its methods.
 * - @warning Tests specifically proving contributions to FE-0030 are not present within this module, and documentation linking this module to FE-0030 is missing.
 *
 * @par Contribution to FE-0070.1 (Logger):
 * - Implements the ILogger component [FE-0070.1.1].
 * - Provides `QueryLogMessageKind` which translates log message kind names to IDs [FE-0070.1.3].
 * - The `Log` method implements message logging to stdout [FE-0070.1.6].
 * - Supports default Log Message Kinds (Debug, Error, Warning, Event, Information) as defined in Table 5-7 [FE-0070.1.4].
 * - @warning The mapping of Log Message Kinds is hardcoded and not persisted as required by [FE-0070.1.5].
 * - @warning The explicit maintenance of a mapping of Log Message Kinds as per [FE-0070.1.2] is handled via hardcoded logic in `QueryLogMessageKind`.
 */
#pragma once

// SMP interfaces, core object base, and standard I/O + timed mutex primitives.
#include <Smp/IObject.h>
#include <Smp/Services/ILogger.h>
#include <core/Object.hpp>
#include <iostream>
#include <mutex>

namespace utils {

/**
 * @brief Implementation of the SMP Logger Service.
 * @details Provides message logging capabilities to stdout. Concurrent calls to `Log`
 * serialize output via `_mutex` (`std::timed_mutex`) with a bounded 1-second wait so
 * logging contention cannot deadlock simulation threads.
 *
 * @task TSK-20261113-003
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.45, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed SMP `ILogger` and `IComponent` service boundary.
 * @compliance Uses `std::timed_mutex` for bounded synchronization, null-checks incoming
 * C-strings, and maintains `const` correctness on inspection methods.
 * @invariant `_mutex` serializes all formatted writes to `std::cout` within `Log()`.
 *
 * @note This class is thread-safe using std::timed_mutex.
 *       It does not implement NamedObject features as defined in FE-0020. Specifically, it does not provide
 *       a NamedObject class, parent/child management, or tree/graph traversal utilities as described in FE-0020.1, FE-0020.4, FE-0020.12, etc.
 */
class Logger : public core::Object, public virtual Smp::Services::ILogger {
public:
  /**
   * @brief Default constructor.
   * @details Initializes the base `core::Object` with component name `"Logger"` and
   * description `"SMP Logger Service"`.
   *
   * @task TSK-20261113-003
   * @feature FE-0070
   * @constraint CS-0010.45, CS-0090.1, CS-0090.2
   * @compliance Default-constructs `_mutex` and initializes `core::Object` metadata.
   * @post Component is constructed and ready to handle `QueryLogMessageKind` and `Log` calls.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if `core::Object` string metadata allocation fails.
   * @error
   *   - Resource / Allocation (`std::bad_alloc`): Construction aborts with no partially leaked members.
   * @containment
   *   Approach: Member initialization list constructs `core::Object` and `_mutex` cleanly via RAII.
   *   Limitations: Heap exhaustion during `core::Object` name/description construction propagates to caller.
   */
  Logger();

  /**
   * @brief Virtual destructor.
   * @details Destroys `Logger` and releases `_mutex` via RAII without throwing.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0060.5, CS-0090.1, CS-0090.2
   * @compliance Marked `noexcept` and `override` with defaulted RAII cleanup.
   */
  virtual ~Logger() noexcept override = default;

  // IComponent methods

  /**
   * @brief Query the current SMP component lifecycle state.
   * @details Returns the fixed connected state `CSK_Connected` for this utility service.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetState` interface.
   * @compliance Const-qualified state query returning an explicit SMP enumeration value.
   * @post Returns `Smp::ComponentStateKind::CSK_Connected` without mutating `*this`.
   * @return Smp::ComponentStateKind
   *   Justification: Exposes component lifecycle state required by `Smp::IComponent`.
   *   Usage: Caller inspects enum value to confirm logger readiness.
   *   Consumed: Value returned by copy; no internal state consumed.
   *   Prerequisites: Valid `Logger` instance.
   *   Limitations: Always reports `CSK_Connected` regardless of lifecycle transitions.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or assign locally; no cleanup needed.
   *   Specific Values: Always `Smp::ComponentStateKind::CSK_Connected`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw return path): Always returns `CSK_Connected`.
   * @containment
   *   Approach: Pure constant-returning accessor performs no locking or I/O.
   *   Limitations: Virtual ABI omits `noexcept`; lifecycle state transitions are stubbed.
   */
  Smp::ComponentStateKind GetState() const override;

  /**
   * @brief Publish component properties to an SMP publication receiver.
   * @details Guards against a null `receiver` pointer and returns immediately because
   * `Logger` does not publish SMP fields or operations.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Publish` interface.
   * @compliance Explicitly validates `receiver` for `nullptr` before no-op return.
   * @param[in] receiver
   *   Justification: Required by `Smp::IComponent::Publish` to expose published component members.
   *   Usage: Checked against `nullptr` and unused because no items are published.
   *   Consumed: No (non-owning pointer; neither stored nor dereferenced).
   *   Prerequisites: May be `nullptr` (safely ignored) or a valid `Smp::IPublication *`.
   * @pre `true` (null pointer is handled safely).
   * @post `*this` remains unchanged.
   * @error
   *   - Validation / Contract (`receiver == nullptr`): Safely ignored via early return; no-throw.
   * @containment
   *   Approach: Null guard prevents invalid dereference on future publication extensions.
   *   Limitations: Does not publish log counters or configuration parameters.
   */
  void Publish(Smp::IPublication *receiver) override;

  /**
   * @brief Configure the component with logger and link registry services.
   * @details No-op override of `Smp::IComponent::Configure`; parameters are ignored
   * because `Logger` is a standalone stdout logging service.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Configure` interface.
   * @compliance Satisfies `Smp::IComponent::Configure` safely without mutating state.
   * @param[in] logger
   *   Justification: Required by `Smp::IComponent::Configure` to inject a parent logger.
   *   Usage: Unused; this component is itself the root logger service.
   *   Consumed: No (non-owning pointer; not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILogger *`.
   * @param[in] linkRegistry
   *   Justification: Required by `Smp::IComponent::Configure` to inject a link registry.
   *   Usage: Unused in this stub implementation.
   *   Consumed: No (non-owning pointer; not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::Services::ILinkRegistry *`.
   * @pre `true` (null pointers tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: No-op body isolates configuration calls from output stream state.
   *   Limitations: Does not validate arguments or attach external logging sinks.
   */
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override;

  /**
   * @brief Connect the component to the hosting SMP simulator.
   * @details No-op override of `Smp::IComponent::Connect`; the simulator pointer is
   * neither retained nor dereferenced.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::Connect` interface.
   * @compliance Implements `Smp::IComponent::Connect` as a safe no-op.
   * @param[in] simulator
   *   Justification: Required by `Smp::IComponent::Connect` to bind to the simulator.
   *   Usage: Unused in this stub implementation.
   *   Consumed: No (non-owning pointer; not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::ISimulator *`.
   * @pre `true` (null pointer tolerated safely).
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: No-op body avoids coupling logger lifecycle to simulator state.
   *   Limitations: Simulator services cannot be queried later via stored pointer.
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
   * @compliance Safe no-op disconnect leaving stdout logging operational.
   * @post `*this` remains unchanged.
   * @error
   *   - None (no-throw stub): Never fails or throws.
   * @containment
   *   Approach: Stateless no-op allows late shutdown diagnostics to continue safely.
   *   Limitations: Does not flush or close external log sinks.
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
   * @compliance Uses a function-local static UUID and returns a `const` reference.
   * @post Returned reference refers to valid static `Smp::Uuid` `{0, 0, 0, 0, 3}`.
   * @return const Smp::Uuid &
   *   Justification: Exposes the component UUID required by `Smp::IComponent::GetUuid`.
   *   Usage: Caller inspects or copies the UUID for service lookup.
   *   Consumed: Borrowed const reference; ownership remains with static storage.
   *   Prerequisites: None.
   *   Limitations: Uses a generic placeholder UUID `{0, 0, 0, 0, 3}`.
   *   Lifecycle: Static storage duration; valid for the lifetime of the process.
   *   Modification / Deletion: Immutable (`const &`); caller MUST NOT cast away const or delete.
   *   Specific Values: Reference to `{0, 0, 0, 0, 3}`.
   *   Error Behavior: Never fails or throws.
   * @error
   *   - None (no-throw constant reference): Always returns valid reference.
   * @containment
   *   Approach: Function-local static avoids global initialization order hazards.
   *   Limitations: Base virtual declaration omits `noexcept`.
   */
  const Smp::Uuid &GetUuid() const override;

  /**
   * @brief Retrieve a published field by full name.
   * @details Stub override returning `nullptr` because `Logger` exposes no SMP fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetField` interface.
   * @compliance Const-qualified stub returning `nullptr` safely.
   * @param[in] fullName
   *   Justification: Required by `Smp::IComponent::GetField` to specify the field path.
   *   Usage: Ignored; no fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely).
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
   *   Error Behavior: Never fails or throws.
   * @retval nullptr `Logger` publishes no fields.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids dereferencing `fullName`.
   *   Limitations: Field reflection is unsupported.
   */
  Smp::IField *GetField(Smp::String8 fullName) const override;

  /**
   * @brief Retrieve the collection of all published fields.
   * @details Stub override returning `nullptr` because `Logger` publishes no fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetFields` interface.
   * @compliance Const-qualified override returning `nullptr` for empty field set.
   * @post Returns `nullptr` without modifying `*this`.
   * @return const Smp::FieldCollection *
   *   Justification: Satisfies `Smp::IComponent::GetFields` when no fields are published.
   *   Usage: Caller checks for `nullptr` before traversing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must check for `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Immutable null pointer; caller MUST NOT dereference or delete.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never fails or throws.
   * @retval nullptr No field collection exists.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Direct null return without allocation.
   *   Limitations: Callers must handle `nullptr` cleanly.
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
   * @compliance Const-qualified override returning initialized `Smp::AnySimple` by value.
   * @param[in] fullName
   *   Justification: Required by `Smp::IComponent::GetSimpleValue` to identify the field.
   *   Usage: Ignored; no simple fields exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @pre `true` (all inputs tolerated safely).
   * @post Returns default `Smp::AnySimple()` without altering `*this`.
   * @return Smp::AnySimple
   *   Justification: Satisfies `Smp::IComponent::GetSimpleValue` return type.
   *   Usage: Caller checks `.type` before accessing payload.
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
   *   Approach: Returns default value without accessing `fullName`.
   *   Limitations: Does not throw on unknown field names.
   */
  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;

  /**
   * @brief Set the simple value of a named field.
   * @details Stub override; performs no action because `Logger` has no simple fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleValue` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::SetSimpleValue`.
   * @param[in] fullName
   *   Justification: Identifies the target field path in `Smp::IComponent`.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] value
   *   Justification: Supplies the value to assign in `Smp::IComponent`.
   *   Usage: Ignored in this stub.
   *   Consumed: Scalar/variant copied and discarded.
   *   Prerequisites: Any valid `Smp::AnySimple`.
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
   * @details Stub override; performs no action because `Logger` has no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::GetSimpleArrayValue` interface.
   * @compliance Const-qualified no-op stub preserving caller buffer.
   * @param[in] fullName
   *   Justification: Identifies the array field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Requested number of elements.
   *   Usage: Ignored in this stub.
   *   Consumed: Copied and unused.
   *   Prerequisites: Any `Smp::UInt64`.
   * @param[out] values
   *   Justification: Caller destination buffer for array elements.
   *   Usage: Left untouched by this stub.
   *   Consumed: No (buffer not modified).
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
   *   Approach: No-op body avoids buffer writes when no fields exist.
   *   Limitations: Does not populate `values` or report missing field errors.
   */
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;

  /**
   * @brief Write an array slice to a named simple array field.
   * @details Stub override; performs no action because `Logger` has no array fields.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::SetSimpleArrayValue` interface.
   * @compliance Safe no-op stub for `Smp::IComponent::SetSimpleArrayValue`.
   * @param[in] fullName
   *   Justification: Identifies the target array field path.
   *   Usage: Ignored in this stub.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any null-terminated string.
   * @param[in] length
   *   Justification: Number of elements in `values`.
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
   * @details Stub override returning `false` because `Logger` manages no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::AddChild` interface.
   * @compliance Safely rejects child insertion with `false`.
   * @param[in] child
   *   Justification: Child object candidate to attach.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (not stored or dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Target collection pointer.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Signals whether child insertion succeeded.
   *   Usage: Caller inspects boolean result.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `Logger` does not support child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Immediately returns `false` without mutating state.
   *   Limitations: Dynamic child composition is unavailable.
   */
  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::ICollectionBase *collection) override;

  /**
   * @brief Attempt to remove a dynamic child object from a collection.
   * @details Stub override returning `false` because `Logger` manages no child collections.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `IComponent::RemoveChild` interface.
   * @compliance Safely rejects child removal with `false`.
   * @param[in] child
   *   Justification: Child object candidate to detach.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `Smp::IObject *`.
   * @param[in] collection
   *   Justification: Target collection pointer.
   *   Usage: Ignored; no child collections exist.
   *   Consumed: No (not dereferenced).
   *   Prerequisites: May be `nullptr` or any `const Smp::ICollectionBase *`.
   * @pre `true` (all inputs safely ignored).
   * @post Returns `false` and `*this` remains unchanged.
   * @return Smp::Bool
   *   Justification: Signals whether child removal succeeded.
   *   Usage: Caller inspects boolean result.
   *   Consumed: Scalar boolean returned by copy.
   *   Prerequisites: None.
   *   Limitations: Always returns `false`.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller may copy or discard freely.
   *   Specific Values: Always `false`.
   *   Error Behavior: Never throws; returns `false`.
   * @retval false `Logger` owns no child objects.
   * @error
   *   - None (no-throw rejection): Always returns `false`.
   * @containment
   *   Approach: Stateless `false` return prevents dangling collection access.
   *   Limitations: Does not report detailed diagnostics.
   */
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::ICollectionBase *collection) override;

  /**
   * @brief Check whether a named child belongs to a given collection.
   * @details Stub override returning `nullptr` because `Logger` holds no child collections.
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
   *   Justification: Returns matching child pointer or `nullptr` if absent.
   *   Usage: Caller checks for `nullptr` before dereferencing.
   *   Consumed: Borrowed null sentinel; no ownership transferred.
   *   Prerequisites: Caller must guard against `nullptr`.
   *   Limitations: Always returns `nullptr`.
   *   Lifecycle: Not applicable (`nullptr`).
   *   Modification / Deletion: Caller MUST NOT dereference or delete `nullptr`.
   *   Specific Values: Always `nullptr`.
   *   Error Behavior: Never throws; returns `nullptr`.
   * @retval nullptr Child is not present in any collection.
   * @error
   *   - None (no-throw stub): Always returns `nullptr`.
   * @containment
   *   Approach: Stateless `nullptr` return avoids unsafe pointer traversal.
   *   Limitations: Child membership queries always yield `nullptr`.
   */
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::ICollectionBase *collection) const override;

  /**
   * @brief Retrieve a child object by name.
   * @details Stub override returning `nullptr` because `Logger` is a leaf service.
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

  // ILogger methods

  /**
   * @brief Query the log message kind ID for a given name.
   * @details Compares `messageKindName` against the five predefined SMP log kind strings
   * (`LMK_InformationName`, `LMK_EventName`, `LMK_WarningName`, `LMK_ErrorName`, `LMK_DebugName`)
   * using `std::strcmp` and returns the matching `LogMessageKind` constant, or `-1` if null or unknown.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
   * @exposed SMP `ILogger::QueryLogMessageKind` interface.
   * @compliance Checks for `nullptr` before `std::strcmp` and returns explicit sentinel `-1` on mismatch.
   * @param[in] messageKindName
   *   Justification: Supplies the symbolic SMP log kind name to resolve to a numeric ID.
   *   Usage: Guarded against `nullptr` and compared against standard SMP `LMK_*Name` constants.
   *   Consumed: No (read-only C-string pointer; not retained).
   *   Prerequisites: Null-terminated C-string (or `nullptr`, which safely returns `-1`).
   * @pre `messageKindName == nullptr || messageKindName` is null-terminated.
   * @post Returns standard `Smp::Services::ILogger::LMK_*` constant on exact match, or `-1` otherwise; `*this` unchanged.
   * @return Smp::Services::LogMessageKind
   *   Justification: Translates human-readable log kind names into numeric IDs for `Log()`.
   *   Usage: Caller checks for `-1` before passing unknown custom IDs to `Log()`.
   *   Consumed: Scalar integer returned by copy; caller owns its copy.
   *   Prerequisites: None.
   *   Limitations: Hardcoded to the 5 predefined SMP kinds; dynamic custom kind registration [FE-0070.1.5] is not implemented.
   *   Lifecycle: Standalone temporary prvalue.
   *   Modification / Deletion: Caller owns scalar copy; no cleanup required.
   *   Specific Values: `LMK_Information`, `LMK_Event`, `LMK_Warning`, `LMK_Error`, `LMK_Debug`, or `-1` (null/unknown).
   *   Error Behavior: Returns `-1` for null or unrecognized names; does not throw.
   * @retval -1 `messageKindName` is `nullptr` or does not match a predefined SMP log kind.
   * @error
   *   - Validation / Contract (`messageKindName == nullptr` or unknown): Returns `-1` safely; no-throw.
   * @containment
   *   Approach: Null check precedes `std::strcmp`, and fixed literal comparisons bound runtime complexity.
   *   Limitations: Custom log message kinds cannot be dynamically registered and always resolve to `-1`.
   */
  Smp::Services::LogMessageKind
  QueryLogMessageKind(Smp::String8 messageKindName) override;

  /**
   * @brief Log a message.
   * @details Formats and prints a log line to `std::cout` containing the mapped severity tag
   * (`INFO`, `EVENT`, `WARN`, `ERROR`, `DEBUG`, or `UNKNOWN`), optional `[sender->GetName()]`,
   * and `message`, serialized under `_mutex` with a 1-second timed wait.
   *
   * @task TSK-20261113-003
   * @feature FE-0070, FE-0080
   * @constraint CS-0010.15, CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
   * @exposed SMP `ILogger::Log` interface.
   * @compliance Validates `message != nullptr`, bounds mutex wait to 1 second via `std::timed_mutex`,
   * and handles unknown `kind` values via an explicit `default` switch branch.
   * @param[in] sender
   *   Justification: Identifies the SMP object originating the log message.
   *   Usage: If non-null, `sender->GetName()` is printed as `[name] ` prefix.
   *   Consumed: No (non-owning pointer; not stored beyond call).
   *   Prerequisites: Either `nullptr` or a valid `const Smp::IObject *` whose `GetName()` returns valid text.
   * @param[in] message
   *   Justification: Carries the null-terminated diagnostic message text to output.
   *   Usage: Checked against `nullptr` and streamed to `std::cout` under `_mutex`.
   *   Consumed: No (read-only C-string pointer; not stored).
   *   Prerequisites: If `nullptr`, call returns immediately as a safe no-op; otherwise null-terminated string.
   * @param[in] kind
   *   Justification: Selects the severity prefix tag (`INFO`, `EVENT`, `WARN`, `ERROR`, `DEBUG`, `UNKNOWN`).
   *   Usage: Switched exhaustively with `default` fallback to `"UNKNOWN"`.
   *   Consumed: No (scalar value copied; defaults to `0`).
   *   Prerequisites: Any `Smp::Services::LogMessageKind` integer value.
   * @pre `message != nullptr` for output to be emitted; `sender == nullptr || sender` is valid.
   * @post If `message != nullptr` and `_mutex` acquired within 1s, formatted log line is flushed to `std::cout`.
   * @invariant `_mutex` prevents interleaved log lines across concurrent threads.
   * @throws std::ios_base::failure [Hardware / Bus / I/O] Propagated only if `std::cout` stream exceptions are enabled by caller.
   * @throws std::bad_alloc [Resource / Allocation] Propagated if constructing `kindStr` fails under memory exhaustion.
   * @error
   *   - Validation / Contract (`message == nullptr`): Returns early without locking or printing; no-throw.
   *   - Transient / Timeout (`_mutex` contention > 1s): Drops log line and returns cleanly to avoid blocking caller; no-throw.
   *   - Hardware / Bus / I/O (`std::cout` stream error): Handled per standard stream error flags; RAII unlocks `_mutex`.
   * @containment
   *   Approach: Null message check precedes locking, timed mutex bounds maximum stall to 1s, and `default`
   *   switch case prevents uninitialized tag strings.
   *   Limitations: Log messages are silently dropped if `_mutex` contention exceeds 1 second; assumes `sender->GetName()`
   *   is non-null when `sender != nullptr`.
   */
  void Log(const Smp::IObject *sender, Smp::String8 message,
           Smp::Services::LogMessageKind kind = 0) override;

private:
  /**
   * @brief Mutex for thread-safe logging.
   * @details Serializes concurrent `Log()` formatting and `std::cout` writes using a 1-second timed wait.
   *
   * @task TSK-20261113-003
   * @feature FE-0030, FE-0070, FE-0080
   * @constraint CS-0010.26, CS-0010.46, CS-0090.1, CS-0090.2
   * @compliance Timed mutex ensuring bounded blocking during multi-threaded logging.
   */
  std::timed_mutex _mutex;
};

} // namespace utils
