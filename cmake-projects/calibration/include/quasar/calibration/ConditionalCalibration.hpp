/**
 * @file ConditionalCalibration.hpp
 * @brief Declares ConditionalCalibration for runtime predicate-dispatched calibration selection.
 *
 * @details Evaluates a user-supplied condition callback (`m_conditionEvaluator`) on each
 * conversion and routes `rawToEng` / `engToRaw` to the matching `ICalibration` case or
 * fallback `m_defaultCase`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Documents every member, constructor, override, and dispatch branch per CS-0090.
 */

#ifndef QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
#define QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP

// NamedCalibration base class and standard map/functional utilities.
#include "NamedCalibration.hpp"
#include <map>
#include <functional>

/**
 * @brief Quasar engineering-unit calibration namespace.
 *
 * @details Encapsulates `ConditionalCalibration` alongside hierarchical calibration nodes.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Contains `ConditionalCalibration` within `quasar::calibration`.
 */
namespace quasar::calibration {

/**
 * @class ConditionalCalibration
 * @brief Calibration that selects an underlying calibration based on a condition.
 *
 * @details Inherits from `NamedCalibration` (`NamedObject` + `ICalibration`) and dispatches
 * `rawToEng` / `engToRaw` dynamically by evaluating `m_conditionEvaluator` and looking up
 * the resulting `Variant` key in `m_cases`, falling back to `m_defaultCase` or identity.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0010.6, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Case entries and default fallback are managed via `std::shared_ptr<ICalibration>`.
 * @compliance Guards null evaluators and null calibration pointers before dispatching.
 */
class ConditionalCalibration : public NamedCalibration {
private:
    /**
     * @brief Default calibration if no case matches or no evaluator is set.
     * @details Shared pointer to fallback `ICalibration`; pass-through applies if null.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Shared ownership via `std::shared_ptr`.
     */
    std::shared_ptr<ICalibration> m_defaultCase;

    /**
     * @brief Function to evaluate the condition at conversion time.
     * @details Returns a `Variant` key matched against `m_cases`.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Explicit `std::function<Variant()>` callback member.
     */
    std::function<Variant()> m_conditionEvaluator;

    /**
     * @brief Map of conditions to calibrations.
     * @details Associates discrete `Variant` condition outcomes with target `ICalibration` instances.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed `std::map` of `std::shared_ptr<ICalibration>` values.
     */
    std::map<Variant, std::shared_ptr<ICalibration>> m_cases;
    
public:
    /**
     * @brief Constructs a ConditionalCalibration.
     *
     * @details Initializes base `NamedCalibration` with `name` and empty evaluator/case map.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` constructor forwarding node name to `NamedCalibration`.
     *
     * @param[in] name
     *   Justification: Assigns the unique `NamedObject` identifier for this conditional node.
     *   Usage: Passed to `NamedCalibration(name)` base constructor.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name string.
     *
     * @pre !name.empty()
     * @post Object constructed with null `m_defaultCase`, empty `m_conditionEvaluator`, and empty `m_cases`.
     * @throws std::exception [Resource / Allocation] Propagates if `NamedObject` construction throws.
     * @error
     *   - Resource / Allocation (name allocation OOM): Propagates cleanly.
     * @containment
     *   Approach: Base initializer delegates directly to `NamedCalibration`.
     *   Limitations: Caller must bind evaluator/cases prior to non-identity use.
     */
    explicit ConditionalCalibration(const std::string& name) : NamedCalibration(name) {}

    /**
     * @brief Returns the type of the object.
     *
     * @details Overrides `NamedObject::getType()` with `"ConditionalCalibration"`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Deterministic reflexive type tag.
     *
     * @post Returned string equals `"ConditionalCalibration"`.
     * @return std::string
     *   Justification: Supports Quasar reflexive type identification.
     *   Usage: Caller compares returned string against `"ConditionalCalibration"`.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"ConditionalCalibration"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under OOM if SSO is unavailable.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Returns constant string literal.
     *   Limitations: Non-`noexcept` due to base class signature.
     */
    std::string getType() const override { return "ConditionalCalibration"; }

    /**
     * @brief Creates a copy of this `ConditionalCalibration` copying evaluator, default, and cases.
     *
     * @details Allocates a new `ConditionalCalibration` with the same name via `std::make_shared`
     * and copies `m_defaultCase`, `m_conditionEvaluator`, and `m_cases`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `std::make_shared` instead of raw `new`.
     *
     * @param[in] policy
     *   Justification: Required by `NamedObject::clone` override signature.
     *   Usage: Unused by shallow case-map duplication (`DUPLICATE` semantics).
     *   Consumed: Yes (copied enum value).
     *   Prerequisites: Valid `quasar::named::CopyPolicy` enumerator.
     *
     * @pre true
     * @post Returned `shared_ptr` is non-null and shares references to the same case calibrations.
     * @return std::shared_ptr<NamedObject>
     *   Justification: Enables polymorphic duplication inside Quasar object trees.
     *   Usage: Caller receives independent `ConditionalCalibration` node sharing case targets.
     *   Consumed: Ownership transferred via `std::shared_ptr`.
     *   Prerequisites: None.
     *   Limitations: Copies `shared_ptr` handles to underlying `ICalibration` cases rather than deep-cloning them; `policy` ignored.
     *   Lifecycle: Managed by returned `std::shared_ptr`.
     *   Modification / Deletion: Caller may modify returned clone independently of `*this`.
     *   Specific Values: Non-null `std::shared_ptr<NamedObject>` wrapping `ConditionalCalibration`.
     *   Error Behavior: Throws `std::bad_alloc` if allocation fails.
     * @throws std::bad_alloc [Resource / Allocation] Thrown on heap exhaustion.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`; source `*this` untouched.
     * @containment
     *   Approach: `std::make_shared` ensures exception-safe allocation.
     *   Limitations: Shallow-copies `m_cases` shared pointers regardless of `policy`.
     */
    std::shared_ptr<NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        // Allocate cloned ConditionalCalibration via std::make_shared [CS-0010.10].
        std::shared_ptr<ConditionalCalibration> comp = std::make_shared<ConditionalCalibration>(getName());
        comp->m_defaultCase = m_defaultCase;
        comp->m_conditionEvaluator = m_conditionEvaluator;
        // Copy case lookup map (shallow shared_ptr handles).
        comp->m_cases = m_cases;
        return comp;
    }

    /**
     * @brief Sets the condition evaluator function.
     *
     * @details Moves `eval` into `m_conditionEvaluator`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Transfers callable via `std::move`.
     *
     * @param[in] eval
     *   Justification: Supplies runtime predicate producing a `Variant` case selector.
     *   Usage: Moved into `m_conditionEvaluator` and invoked during conversions.
     *   Consumed: Yes (moved-from via `std::move`).
     *   Prerequisites: Valid `std::function<Variant()>` (empty disables case lookup and triggers default fallback).
     *
     * @pre true
     * @post `m_conditionEvaluator` holds `eval`.
     * @error
     *   - Validation / Contract (empty `eval`): Conversions fall back cleanly to `m_defaultCase` or pass-through.
     * @containment
     *   Approach: `std::move` assignment with null-callable guard in `rawToEng`/`engToRaw`.
     *   Limitations: Not thread-safe against concurrent `rawToEng`/`engToRaw` calls.
     */
    void setEvaluator(std::function<Variant()> eval) { m_conditionEvaluator = std::move(eval); }

    /**
     * @brief Adds a calibration case for a specific condition value.
     *
     * @details Inserts or updates `m_cases[val] = cal`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Stores shared calibration handles safely in `std::map`.
     *
     * @param[in] val
     *   Justification: Specifies the condition selector key returned by `m_conditionEvaluator`.
     *   Usage: Used as lookup key in `m_cases`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Valid `Variant` key comparable via `std::less<Variant>`.
     * @param[in] cal
     *   Justification: Provides the calibration strategy active when `m_conditionEvaluator() == val`.
     *   Usage: Stored in `m_cases[val]`.
     *   Consumed: No (shared_ptr copied into map).
     *   Prerequisites: Non-null `std::shared_ptr<ICalibration>` (null falls back to default).
     *
     * @pre true
     * @post `m_cases[val] == cal`.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if map node insertion allocates and fails.
     * @error
     *   - Resource / Allocation (map insertion OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Null case pointers are guarded at dispatch time in `rawToEng`/`engToRaw`.
     *   Limitations: Overwrites any previously registered case with the same `Variant` key.
     */
    void addCase(const Variant& val, std::shared_ptr<ICalibration> cal) { m_cases[val] = cal; }

    /**
     * @brief Sets the default calibration.
     *
     * @details Assigns `m_defaultCase = cal` used when no evaluator is configured or no case matches.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Retains default calibration via `std::shared_ptr`.
     *
     * @param[in] cal
     *   Justification: Supplies fallback calibration when condition does not match `m_cases`.
     *   Usage: Assigned to `m_defaultCase`.
     *   Consumed: No (shared_ptr copied into `m_defaultCase`).
     *   Prerequisites: May be `nullptr` (enables raw pass-through fallback); otherwise valid `ICalibration`.
     *
     * @pre true
     * @post `m_defaultCase == cal`.
     * @error
     *   - Validation / Contract (null `cal`): Falls back cleanly to identity pass-through.
     * @containment
     *   Approach: Null-checked `m_defaultCase` dereference in `rawToEng`/`engToRaw`.
     *   Limitations: Must not be called concurrently with active conversions without synchronization.
     */
    void setDefaultCase(std::shared_ptr<ICalibration> cal) { m_defaultCase = cal; }

    /**
     * @brief Transforms raw input to engineering output via the currently active condition branch.
     *
     * @details Evaluates `m_conditionEvaluator` if present, dispatches to matching `m_cases`
     * calibration, or falls back to `m_defaultCase` (or identity pass-through if null).
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards empty evaluator, missing case keys, and null calibration pointers before dispatch.
     *
     * @param[in] raw
     *   Justification: Supplies raw input value to route through the selected calibration.
     *   Usage: Forwarded to matched case calibration, `m_defaultCase`, or returned unchanged.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Compatible with the selected underlying `ICalibration`.
     *
     * @pre true (null evaluator/default degrade cleanly to identity pass-through).
     * @post Returned `Variant` holds transformed value from selected calibration (or `raw` pass-through).
     * @return Variant
     *   Justification: Provides dynamically selected engineering-unit result.
     *   Usage: Caller unwraps expected `Variant` alternative.
     *   Consumed: Value returned by move/copy.
     *   Prerequisites: None.
     *   Limitations: Output alternative depends on whichever branch matched at evaluation time.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: Selected branch output or `raw` pass-through.
     *   Error Behavior: Propagates any exception thrown by `m_conditionEvaluator` or selected `ICalibration`.
     * @throws std::exception [Validation / Contract] Propagates exceptions from `m_conditionEvaluator` or target `rawToEng`.
     * @error
     *   - Validation / Contract (evaluator or selected `ICalibration::rawToEng` throws): Propagates cleanly; `this` untouched.
     * @containment
     *   Approach: Explicit null checks on `m_conditionEvaluator`, `it->second`, and `m_defaultCase` prevent null dereference.
     *   Limitations: Does not catch exceptions raised inside caller-provided `m_conditionEvaluator`.
     */
    Variant rawToEng(const Variant& raw) const override {
        // Fall back to default or identity when no condition evaluator is bound.
        if (!m_conditionEvaluator) {
            // Apply default calibration if configured.
            if (m_defaultCase) return m_defaultCase->rawToEng(raw);
            return raw;
        }
        // Evaluate condition and dispatch to matching non-null case calibration.
        Variant cond = m_conditionEvaluator();
        std::map<Variant, std::shared_ptr<ICalibration>>::const_iterator it = m_cases.find(cond);
        // Invoke matched case calibration when found and non-null.
        if (it != m_cases.end() && it->second) return it->second->rawToEng(raw);
        // Fall back to default calibration if no case matched.
        if (m_defaultCase) return m_defaultCase->rawToEng(raw);
        return raw;
    }

    /**
     * @brief Transforms engineering input back to raw output via the active condition branch.
     *
     * @details Mirrors `rawToEng` branch selection using `engToRaw` on matched case,
     * `m_defaultCase`, or identity pass-through.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards empty evaluator, missing case keys, and null calibration pointers before dispatch.
     *
     * @param[in] eng
     *   Justification: Supplies engineering input value to invert through the selected calibration.
     *   Usage: Forwarded to matched case calibration, `m_defaultCase`, or returned unchanged.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Compatible with the inverse mapping of the selected `ICalibration`.
     *
     * @pre true
     * @post Returned `Variant` holds inverse raw value from selected calibration (or `eng` pass-through).
     * @return Variant
     *   Justification: Provides dynamically selected inverse raw result.
     *   Usage: Caller unwraps expected raw `Variant` alternative.
     *   Consumed: Value returned by move/copy.
     *   Prerequisites: Selected branch must support `engToRaw`.
     *   Limitations: Selected branch must match the condition active during forward transform for consistent round-tripping.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: Selected branch inverse output or `eng` pass-through.
     *   Error Behavior: Propagates any exception thrown by `m_conditionEvaluator` or selected `ICalibration::engToRaw`.
     * @throws std::exception [Validation / Contract] Propagates exceptions from `m_conditionEvaluator` or target `engToRaw`.
     * @error
     *   - Validation / Contract (evaluator or selected `ICalibration::engToRaw` throws): Propagates cleanly; `this` untouched.
     * @containment
     *   Approach: Null checks on `m_conditionEvaluator`, `it->second`, and `m_defaultCase` prevent null pointer crashes.
     *   Limitations: Relies on external condition evaluator remaining exception-safe.
     */
    Variant engToRaw(const Variant& eng) const override {
        // Fall back to default or identity when no condition evaluator is bound.
        if (!m_conditionEvaluator) {
            // Apply default calibration inverse if configured.
            if (m_defaultCase) return m_defaultCase->engToRaw(eng);
            return eng;
        }
        // Evaluate condition and dispatch inverse transform to matching case calibration.
        Variant cond = m_conditionEvaluator();
        std::map<Variant, std::shared_ptr<ICalibration>>::const_iterator it = m_cases.find(cond);
        // Invoke matched case inverse when found and non-null.
        if (it != m_cases.end() && it->second) return it->second->engToRaw(eng);
        // Fall back to default calibration inverse if no case matched.
        if (m_defaultCase) return m_defaultCase->engToRaw(eng);
        return eng;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
