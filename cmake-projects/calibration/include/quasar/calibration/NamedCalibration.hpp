/**
 * @file NamedCalibration.hpp
 * @brief Declares NamedObject-backed calibration nodes (Identity, Linear, Polynomial, Composite).
 *
 * @details Combines `quasar::named::NamedObject` hierarchy/serialization support with the
 * `ICalibration` interface so calibrations live inside Quasar trees and store parameters
 * (`scale`, `offset`, `a0..an`, or child calibration pipelines) as inspectable child nodes.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Enforces complete Doxygen coverage across all classes, fields, constructors,
 * destructors, methods, bounded loops, and control structures.
 */

#ifndef QUASAR_CALIBRATION_NAMEDCALIBRATION_HPP
#define QUASAR_CALIBRATION_NAMEDCALIBRATION_HPP

// Quasar NamedObject hierarchy and typed scalar primitives.
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedInteger.hpp"
// Concrete calibration algorithms and standard containers.
#include "quasar/calibration/Calibrations.hpp"
#include <memory>
#include <vector>
#include <map>

/**
 * @brief Quasar engineering-unit calibration namespace.
 *
 * @details Houses hierarchical `NamedCalibration` classes bridging `NamedObject` and `ICalibration`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Scopes named calibration tree nodes inside `quasar::calibration`.
 */
namespace quasar::calibration {

/**
 * @class NamedCalibration
 * @brief Abstract base combining `NamedObject` tree membership with `ICalibration`.
 *
 * @details Allows calibrations to be stored inside `CalibrationManager`, serialized, cloned,
 * and discovered by name while exposing `rawToEng` and `engToRaw`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0060.5, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Inherits `NamedObject` identity and `ICalibration` pure virtual conversion interface.
 * @compliance Provides explicit constructor and defaulted virtual destructor.
 */
class NamedCalibration : public quasar::named::NamedObject, public ICalibration {
public:
    /**
     * @brief Constructs a NamedCalibration node with the given object name.
     *
     * @details Forwards `name` to `quasar::named::NamedObject(name)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` single-argument constructor.
     *
     * @param[in] name
     *   Justification: Assigns the hierarchical `NamedObject` name for tree lookup.
     *   Usage: Forwarded to `NamedObject(name)` base initializer.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     *
     * @pre !name.empty()
     * @post Node initialized with `getName() == name`.
     * @throws std::exception [Resource / Allocation] Propagates if `NamedObject` construction throws.
     * @error
     *   - Resource / Allocation (name allocation OOM): Propagates cleanly.
     * @containment
     *   Approach: Delegates directly to `NamedObject` constructor.
     *   Limitations: Subclasses must still implement `rawToEng` and `engToRaw`.
     */
    explicit NamedCalibration(const std::string& name) : NamedObject(name) {}

    /**
     * @brief Defaulted virtual destructor for safe polymorphic cleanup.
     *
     * @details Ensures derived `NamedCalibration` subclasses tear down child nodes cleanly.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0060.5, CS-0090.1, CS-0090.2
     * @exposed
     * @compliance Defaulted virtual destructor.
     */
    virtual ~NamedCalibration() = default;
};

/**
 * @class NamedIdentityCalibration
 * @brief `NamedCalibration` wrapping `IdentityCalibration` for pass-through tree nodes.
 *
 * @details Returns raw and engineering variants unmodified while participating in Quasar tree
 * hierarchy, cloning, and type reflection.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Delegates conversions to internal `IdentityCalibration` value member `m_cal`.
 * @compliance Composition over unnecessary logic duplication.
 */
class NamedIdentityCalibration : public NamedCalibration {
private:
    /**
     * @brief Embedded stateless identity calibration delegate.
     * @details Used by `rawToEng` and `engToRaw`.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Value member obeying Rule of Zero.
     */
    IdentityCalibration m_cal;
public:
    /**
     * @brief Constructs a NamedIdentityCalibration node.
     *
     * @details Forwards `name` to `NamedCalibration(name)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` single-argument constructor.
     *
     * @param[in] name
     *   Justification: Assigns the `NamedObject` identifier for this identity calibration node.
     *   Usage: Passed to `NamedCalibration(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     *
     * @pre !name.empty()
     * @post Node constructed with `getName() == name`.
     * @throws std::exception [Resource / Allocation] Propagates if `NamedCalibration` throws.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: Delegates to `NamedCalibration`.
     *   Limitations: None.
     */
    explicit NamedIdentityCalibration(const std::string& name) : NamedCalibration(name) {}
    
    /**
     * @brief Returns the reflexive type identifier `"NamedIdentityCalibration"`.
     *
     * @details Overrides `NamedObject::getType()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Returns deterministic class name.
     *
     * @post Returned string equals `"NamedIdentityCalibration"`.
     * @return std::string
     *   Justification: Identifies node type in reflexive serialization/dispatch.
     *   Usage: Caller inspects type string.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"NamedIdentityCalibration"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Constant literal string return.
     *   Limitations: Non-`noexcept` due to base virtual signature.
     */
    std::string getType() const override { return "NamedIdentityCalibration"; }
    
    /**
     * @brief Clones this `NamedIdentityCalibration` node.
     *
     * @details Creates a new `NamedIdentityCalibration` with the same name via `std::make_shared`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Allocates via `std::make_shared`.
     *
     * @param[in] policy
     *   Justification: Required by `NamedObject::clone` override signature.
     *   Usage: Ignored because `NamedIdentityCalibration` is stateless.
     *   Consumed: Yes (copied enum value).
     *   Prerequisites: Valid `CopyPolicy` enum value.
     *
     * @pre true
     * @post Returned `shared_ptr` points to a fresh `NamedIdentityCalibration` named `getName()`.
     * @return std::shared_ptr<quasar::named::NamedObject>
     *   Justification: Supports polymorphic tree duplication.
     *   Usage: Caller inserts cloned node into target parent hierarchy.
     *   Consumed: Ownership transferred via `std::shared_ptr`.
     *   Prerequisites: None.
     *   Limitations: `policy` parameter unused.
     *   Lifecycle: Managed by returned `std::shared_ptr`.
     *   Modification / Deletion: Caller may mutate or drop freely.
     *   Specific Values: Non-null `std::shared_ptr<NamedObject>`.
     *   Error Behavior: Throws `std::bad_alloc` on allocation failure.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared` fails.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: RAII `std::make_shared` allocation.
     *   Limitations: None.
     */
    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedIdentityCalibration>(getName());
    }

    /**
     * @brief Passes raw variant through `m_cal.rawToEng`.
     *
     * @details Delegates directly to `IdentityCalibration::rawToEng`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Preserves input variant unchanged.
     *
     * @param[in] raw
     *   Justification: Supplies raw input variant.
     *   Usage: Forwarded to `m_cal.rawToEng(raw)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Any initialized `Variant`.
     *
     * @pre true
     * @post Returned `Variant` equals `raw`.
     * @return Variant
     *   Justification: Satisfies `ICalibration::rawToEng`.
     *   Usage: Caller unwraps returned variant.
     *   Consumed: Value returned by copy.
     *   Prerequisites: None.
     *   Limitations: Performs no numeric validation.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: Identical to `raw`.
     *   Error Behavior: Propagates `std::bad_alloc` only if `raw` string copy fails.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string variant copy.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: Const delegation to stateless `IdentityCalibration`.
     *   Limitations: None.
     */
    Variant rawToEng(const Variant& raw) const override { return m_cal.rawToEng(raw); }

    /**
     * @brief Passes engineering variant through `m_cal.engToRaw`.
     *
     * @details Delegates directly to `IdentityCalibration::engToRaw`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Preserves input variant unchanged.
     *
     * @param[in] eng
     *   Justification: Supplies engineering input variant.
     *   Usage: Forwarded to `m_cal.engToRaw(eng)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Any initialized `Variant`.
     *
     * @pre true
     * @post Returned `Variant` equals `eng`.
     * @return Variant
     *   Justification: Satisfies `ICalibration::engToRaw`.
     *   Usage: Caller unwraps returned variant.
     *   Consumed: Value returned by copy.
     *   Prerequisites: None.
     *   Limitations: Performs no numeric validation.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: Identical to `eng`.
     *   Error Behavior: Propagates `std::bad_alloc` only if `eng` string copy fails.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string variant copy.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: Const delegation to stateless `IdentityCalibration`.
     *   Limitations: None.
     */
    Variant engToRaw(const Variant& eng) const override { return m_cal.engToRaw(eng); }
};

/**
 * @class NamedLinearCalibration
 * @brief `NamedCalibration` storing `scale` and `offset` as `NamedFloatingPoint<double>` children.
 *
 * @details Exposes linear calibration parameters inside the Quasar object tree for runtime
 * inspection, mutation, and serialization while evaluating transforms via `LinearCalibration`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Child nodes `"scale"` and `"offset"` store `NamedFloatingPoint<double>` parameters.
 * @compliance Uses `NamedFloatingPoint<double>::create` factory methods and validates child types on read.
 */
class NamedLinearCalibration : public NamedCalibration {
public:
    /**
     * @brief Constructs a NamedLinearCalibration with `scale` and `offset` child nodes.
     *
     * @details Creates `"scale"` and `"offset"` `NamedFloatingPoint<double>` children under `*this`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `NamedFloatingPoint::create` factory helpers without raw `new`.
     *
     * @param[in] name
     *   Justification: Identifies this linear calibration node in the Quasar tree.
     *   Usage: Passed to `NamedCalibration(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     * @param[in] scale
     *   Justification: Sets initial linear slope child `"scale"`.
     *   Usage: Stored inside `NamedFloatingPoint<double>` child `"scale"`.
     *   Consumed: Yes (copied scalar `double`).
     *   Prerequisites: Non-zero finite `double` (`scale != 0.0` required by `rawToEng`/`engToRaw`).
     * @param[in] offset
     *   Justification: Sets initial linear offset child `"offset"`.
     *   Usage: Stored inside `NamedFloatingPoint<double>` child `"offset"`.
     *   Consumed: Yes (copied scalar `double`).
     *   Prerequisites: Finite `double`.
     *
     * @pre !name.empty() && scale != 0.0
     * @post Child nodes `"scale"` and `"offset"` exist with specified values.
     * @throws std::exception [Resource / Allocation] Propagated if child creation or attachment fails.
     * @error
     *   - Resource / Allocation (OOM): Propagates exception cleanly.
     * @containment
     *   Approach: Creates parameter children via `std::shared_ptr` factories.
     *   Limitations: Zero `scale` is not rejected at construction here; `LinearCalibration` rejects it on conversion.
     */
    NamedLinearCalibration(const std::string& name, double scale, double offset) 
        : NamedCalibration(name) {
        addChild(quasar::named::NamedFloatingPoint<double>::create("scale", scale));
        addChild(quasar::named::NamedFloatingPoint<double>::create("offset", offset));
    }

    /**
     * @brief Returns the reflexive type string `"NamedLinearCalibration"`.
     *
     * @details Overrides `NamedObject::getType()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Deterministic reflexive type tag.
     *
     * @post Returned string equals `"NamedLinearCalibration"`.
     * @return std::string
     *   Justification: Identifies node type for serialization and Lua/OPC UA dispatch.
     *   Usage: Caller compares returned type tag.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"NamedLinearCalibration"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Constant literal return.
     *   Limitations: Non-`noexcept` due to base virtual signature.
     */
    std::string getType() const override { return "NamedLinearCalibration"; }

    /**
     * @brief Clones this `NamedLinearCalibration` preserving its current `scale` and `offset`.
     *
     * @details Constructs a fresh `NamedLinearCalibration` using `getName()`, `getScale()`, and `getOffset()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Allocates via `std::make_shared`.
     *
     * @param[in] policy
     *   Justification: Required by `NamedObject::clone` override signature.
     *   Usage: Ignored because `scale` and `offset` are deep-copied into new children.
     *   Consumed: Yes (copied enum value).
     *   Prerequisites: Valid `CopyPolicy` enumerator.
     *
     * @pre true
     * @post Returned clone has matching name, scale, and offset children.
     * @return std::shared_ptr<quasar::named::NamedObject>
     *   Justification: Supports tree duplication without sharing mutable parameter children.
     *   Usage: Caller mounts returned node in target tree.
     *   Consumed: Ownership transferred via `std::shared_ptr`.
     *   Prerequisites: None.
     *   Limitations: `policy` argument is unused.
     *   Lifecycle: Managed by returned `std::shared_ptr`.
     *   Modification / Deletion: Caller may mutate cloned parameters independently.
     *   Specific Values: Non-null `std::shared_ptr<NamedObject>`.
     *   Error Behavior: Throws on allocation failure.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared` fails.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: `std::make_shared` ensures exception-safe allocation.
     *   Limitations: Extra children beyond `"scale"` and `"offset"` are not copied.
     */
    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedLinearCalibration>(getName(), getScale(), getOffset());
    }

    /**
     * @brief Returns the scale factor.
     *
     * @details Looks up child `"scale"` and returns its `double` value if it is a
     * `NamedFloatingPoint<double>`, or `1.0` otherwise.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards child existence and dynamic type via `is<NamedFloatingPoint<double>>()`.
     *
     * @post Returns stored `"scale"` child value or safe default `1.0`.
     * @return double
     *   Justification: Exposes configured slope parameter to callers and Lua bindings.
     *   Usage: Used directly for display or `LinearCalibration` construction.
     *   Consumed: Value returned by copy.
     *   Prerequisites: None.
     *   Limitations: Returns `1.0` silently if `"scale"` child was removed or replaced with another type.
     *   Lifecycle: Ephemeral scalar prvalue.
     *   Modification / Deletion: Immutable prvalue; no cleanup required.
     *   Specific Values: Child `"scale"` value or `1.0` fallback.
     *   Error Behavior: No sentinel error; falls back to `1.0` if missing.
     * @throws std::exception [Validation / Contract] Propagates if `getChild` or `as` throws.
     * @error
     *   - Validation / Contract (missing or wrong-typed `"scale"` child): Returns fallback `1.0` safely.
     * @containment
     *   Approach: Null and type checks precede `as<NamedFloatingPoint<double>>()->value()`.
     *   Limitations: Fallback `1.0` masks accidental deletion of the `"scale"` child.
     */
    double getScale() const {
        std::shared_ptr<quasar::named::NamedObject> child = getChild("scale");
        // Extract typed scale child when present and matching NamedFloatingPoint<double>.
        if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
            return child->as<quasar::named::NamedFloatingPoint<double>>()->value();
        }
        return 1.0;
    }
    
    /**
     * @brief Returns the offset.
     *
     * @details Looks up child `"offset"` and returns its `double` value if it is a
     * `NamedFloatingPoint<double>`, or `0.0` otherwise.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards child existence and dynamic type via `is<NamedFloatingPoint<double>>()`.
     *
     * @post Returns stored `"offset"` child value or safe default `0.0`.
     * @return double
     *   Justification: Exposes configured y-intercept parameter to callers and Lua bindings.
     *   Usage: Used directly for display or `LinearCalibration` construction.
     *   Consumed: Value returned by copy.
     *   Prerequisites: None.
     *   Limitations: Returns `0.0` silently if `"offset"` child was removed or wrong-typed.
     *   Lifecycle: Ephemeral scalar prvalue.
     *   Modification / Deletion: Immutable prvalue; no cleanup required.
     *   Specific Values: Child `"offset"` value or `0.0` fallback.
     *   Error Behavior: No sentinel error; falls back to `0.0` if missing.
     * @throws std::exception [Validation / Contract] Propagates if `getChild` or `as` throws.
     * @error
     *   - Validation / Contract (missing or wrong-typed `"offset"` child): Returns fallback `0.0` safely.
     * @containment
     *   Approach: Null and type checks precede `as<NamedFloatingPoint<double>>()->value()`.
     *   Limitations: Fallback `0.0` masks accidental deletion of `"offset"` child.
     */
    double getOffset() const {
        std::shared_ptr<quasar::named::NamedObject> child = getChild("offset");
        // Extract typed offset child when present and matching NamedFloatingPoint<double>.
        if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
            return child->as<quasar::named::NamedFloatingPoint<double>>()->value();
        }
        return 0.0;
    }

    /**
     * @brief Transforms raw variant to engineering variant using current `scale` and `offset`.
     *
     * @details Instantiates a temporary `LinearCalibration(getScale(), getOffset())` and invokes `rawToEng(raw)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates calculation to validated `LinearCalibration`.
     *
     * @param[in] raw
     *   Justification: Supplies raw numeric value to transform.
     *   Usage: Passed to `LinearCalibration::rawToEng`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`) and `getScale() != 0.0`.
     *
     * @pre getScale() != 0.0
     * @post Returned `Variant` holds `double` engineering value.
     * @return Variant
     *   Justification: Delivers calibrated engineering scalar.
     *   Usage: Caller unwraps `double` result.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `raw` and non-zero `scale`.
     *   Limitations: Constructs temporary `LinearCalibration` per call.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` if `getScale() == 0.0` or `raw` is non-numeric.
     * @throws std::invalid_argument [Validation / Contract] Thrown on zero scale or non-numeric `raw`.
     * @error
     *   - Validation / Contract (zero scale or bad `raw` type): Throws `std::invalid_argument`; `this` unchanged.
     * @containment
     *   Approach: `LinearCalibration` enforces non-zero scale and `toDouble` checks variant type.
     *   Limitations: Zero `scale` child fails dynamically at conversion rather than at child mutation.
     */
    Variant rawToEng(const Variant& raw) const override {
        return LinearCalibration(getScale(), getOffset()).rawToEng(raw);
    }

    /**
     * @brief Transforms engineering variant back to raw variant using current `scale` and `offset`.
     *
     * @details Instantiates a temporary `LinearCalibration(getScale(), getOffset())` and invokes `engToRaw(eng)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates inverse calculation to validated `LinearCalibration`.
     *
     * @param[in] eng
     *   Justification: Supplies engineering numeric value to invert.
     *   Usage: Passed to `LinearCalibration::engToRaw`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` and `getScale() != 0.0`.
     *
     * @pre getScale() != 0.0
     * @post Returned `Variant` holds `double` raw value.
     * @return Variant
     *   Justification: Delivers inverted raw scalar.
     *   Usage: Caller unwraps `double` result.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `eng` and non-zero `scale`.
     *   Limitations: Constructs temporary `LinearCalibration` per call.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` if `getScale() == 0.0` or `eng` is non-numeric.
     * @throws std::invalid_argument [Validation / Contract] Thrown on zero scale or non-numeric `eng`.
     * @error
     *   - Validation / Contract (zero scale or bad `eng` type): Throws `std::invalid_argument`; `this` unchanged.
     * @containment
     *   Approach: `LinearCalibration` enforces non-zero scale and `toDouble` checks variant type.
     *   Limitations: Zero `scale` child fails dynamically at conversion time.
     */
    Variant engToRaw(const Variant& eng) const override {
        return LinearCalibration(getScale(), getOffset()).engToRaw(eng);
    }
};

/**
 * @class NamedPolynomialCalibration
 * @brief Named object for polynomial calibration.
 *
 * @details Stores ascending polynomial coefficients as child `NamedFloatingPoint<double>` nodes
 * named `"a0"`, `"a1"`, ... up to a hard bound of 1000 terms, enabling tree inspection, cloning,
 * and Lua binding.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Child nodes `"a0".."a{N-1}"` store consecutive polynomial coefficients (`N <= 1000`).
 * @compliance Enforces hard iteration bound `1000` in both constructor and `getCoeffs()`.
 */
class NamedPolynomialCalibration : public NamedCalibration {
public:
    /**
     * @brief Constructs a NamedPolynomialCalibration.
     *
     * @details Creates `"a0"`, `"a1"`, ... `NamedFloatingPoint<double>` child nodes for each
     * coefficient in `coeffs`, throwing `std::runtime_error` if `coeffs.size() > 1000`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170, FE-0210
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces bounded loop `limit = 1000` over `coeffs`.
     *
     * @param[in] name
     *   Justification: Assigns the unique `NamedObject` identifier in the tree.
     *   Usage: Forwarded to `NamedCalibration(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     * @param[in] coeffs
     *   Justification: Supplies ascending polynomial coefficients `[a0, a1, ...]`.
     *   Usage: Iterated up to `limit = 1000` to create `"a{i}"` child nodes.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Vector with `coeffs.size() <= 1000`.
     *
     * @pre !name.empty() && coeffs.size() <= 1000
     * @post Child nodes `"a0"`..`"a{N-1}"` match `coeffs`.
     * @throws std::runtime_error [Resource / Fatal] Thrown if `coeffs.size() > 1000`.
     * @error
     *   - Resource / Fatal (`coeffs.size() > 1000`): Throws `std::runtime_error` at index `1000`.
     * @containment
     *   Approach: Hard index guard `if (i >= limit)` bounds child creation deterministically.
     *   Limitations: Partial `"a0".."a999"` children exist on `*this` if exception throws during construction.
     */
    NamedPolynomialCalibration(const std::string& name, const std::vector<double>& coeffs) 
        : NamedCalibration(name) {
        const size_t limit = 1000;
        // Create named coefficient children bounded by coeffs.size() and hard limit 1000 [CS-0010.37].
        for (size_t i = 0; i < coeffs.size(); ++i) {
            // Abort construction if coefficient count exceeds hard safety bound.
            if (i >= limit) throw std::runtime_error("Coefficients limit exceeded");
            addChild(quasar::named::NamedFloatingPoint<double>::create("a" + std::to_string(i), coeffs[i]));
        }
    }

    /**
     * @brief Returns the reflexive type string `"NamedPolynomialCalibration"`.
     *
     * @details Overrides `NamedObject::getType()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Deterministic reflexive type tag.
     *
     * @post Returned string equals `"NamedPolynomialCalibration"`.
     * @return std::string
     *   Justification: Identifies node type for serialization and scripting.
     *   Usage: Caller compares returned type string.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"NamedPolynomialCalibration"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Returns short constant literal.
     *   Limitations: Non-`noexcept` due to base virtual signature.
     */
    std::string getType() const override { return "NamedPolynomialCalibration"; }

    /**
     * @brief Clones this `NamedPolynomialCalibration` with its extracted coefficients.
     *
     * @details Calls `getCoeffs()` and allocates a fresh `NamedPolynomialCalibration` via `std::make_shared`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `std::make_shared` and bounded `getCoeffs()`.
     *
     * @param[in] policy
     *   Justification: Required by `NamedObject::clone` override signature.
     *   Usage: Unused; coefficients are re-created as fresh child nodes.
     *   Consumed: Yes (copied enum value).
     *   Prerequisites: Valid `CopyPolicy` enum value.
     *
     * @pre true
     * @post Returned clone contains independent `"a0".."a{N-1}"` children.
     * @return std::shared_ptr<quasar::named::NamedObject>
     *   Justification: Enables duplicating polynomial calibration nodes across trees.
     *   Usage: Caller inserts cloned node into destination parent.
     *   Consumed: Ownership transferred via `std::shared_ptr`.
     *   Prerequisites: None.
     *   Limitations: Ignores non-coefficient extra children and `policy` parameter.
     *   Lifecycle: Managed by returned `std::shared_ptr`.
     *   Modification / Deletion: Caller may mutate cloned node freely.
     *   Specific Values: Non-null `std::shared_ptr<NamedObject>`.
     *   Error Behavior: Propagates `std::runtime_error` from `getCoeffs()` or `std::bad_alloc`.
     * @throws std::runtime_error [Resource / Fatal] Propagated if `getCoeffs()` hits limit.
     * @throws std::bad_alloc [Resource / Allocation] Thrown on allocation failure.
     * @error
     *   - Resource / Fatal (loop limit or OOM): Propagates exception cleanly.
     * @containment
     *   Approach: `std::make_shared` prevents leaks on exception.
     *   Limitations: Non-coefficient children are not cloned.
     */
    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedPolynomialCalibration>(getName(), getCoeffs());
    }

    /**
     * @brief Retrieves coefficients from child objects.
     *
     * @details Iterates sequentially over `"a0"`, `"a1"`, ... up to hard limit `1000`, collecting
     * `NamedFloatingPoint<double>` values until the first missing or differently-typed child.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170, FE-0210
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Replaces unbounded while-true with explicit `limit = 1000` check and throw.
     *
     * @post Returned vector holds contiguous coefficients `[a0, a1, ...]` with `size() <= 1000`.
     * @return std::vector<double>
     *   Justification: Exposes polynomial coefficients to evaluation helpers and Lua bindings.
     *   Usage: Passed to `PolynomialCalibration` or inspected in scripts/tests.
     *   Consumed: Value returned by move/NRVO; caller owns vector.
     *   Prerequisites: None.
     *   Limitations: Stops at first gap in `"a{i}"` naming sequence; capped at 1000 entries.
     *   Lifecycle: Independent caller-owned `std::vector<double>`.
     *   Modification / Deletion: Caller may modify or discard vector freely.
     *   Specific Values: Vector of finite `double` coefficients (empty if `"a0"` absent).
     *   Error Behavior: Throws `std::runtime_error` if `i >= 1000`.
     * @throws std::runtime_error [Resource / Fatal] If loop limit exceeded (`i >= 1000`).
     * @error
     *   - Resource / Fatal (`i >= 1000`): Throws `std::runtime_error`; `this` untouched.
     * @containment
     *   Approach: Hard index check `if (i >= limit)` bounds loop iterations strictly to 1000.
     *   Limitations: Gaps in child indices (`"a0"`, `"a2"`) terminate collection early at `"a1"`.
     */
    std::vector<double> getCoeffs() const {
        std::vector<double> coeffs;
        size_t i = 0;
        const size_t limit = 1000;
        // Scan contiguous "a{i}" children bounded by hard safety limit 1000 [CS-0010.37].
        while (true) {
            // Abort loop if coefficient index reaches hard safety limit.
            if (i >= limit) throw std::runtime_error("Loop limit exceeded in getCoeffs");
            std::shared_ptr<quasar::named::NamedObject> child = getChild("a" + std::to_string(i));
            // Append value when child exists and is NamedFloatingPoint<double>; otherwise stop.
            if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
                coeffs.push_back(child->as<quasar::named::NamedFloatingPoint<double>>()->value());
            } else {
                // First missing or non-double coefficient terminates sequence cleanly.
                break;
            }
            i++;
        }
        return coeffs;
    }

    /**
     * @brief Evaluates the polynomial at `raw` using extracted child coefficients.
     *
     * @details Constructs `PolynomialCalibration(getCoeffs())` and delegates to `rawToEng(raw)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates to bounded `PolynomialCalibration::rawToEng`.
     *
     * @param[in] raw
     *   Justification: Supplies raw numeric input to evaluate polynomial.
     *   Usage: Forwarded to `PolynomialCalibration::rawToEng`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant`.
     *
     * @pre `raw` holds a numeric variant alternative.
     * @post Returned `Variant` holds evaluated `double`.
     * @return Variant
     *   Justification: Delivers calibrated engineering value.
     *   Usage: Caller unwraps `double` result.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `raw` input.
     *   Limitations: Re-reads `"a{i}"` child vector on each call.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` or `std::runtime_error` on invalid input or limit breach.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `raw` is non-numeric.
     * @throws std::runtime_error [Resource / Fatal] Thrown if coefficient limit is exceeded.
     * @error
     *   - Validation / Contract (bad `raw` type): Throws `std::invalid_argument`.
     *   - Resource / Fatal (limit exceeded): Throws `std::runtime_error`.
     * @containment
     *   Approach: Delegates to `getCoeffs()` and `PolynomialCalibration`, both enforcing 1000-item bounds.
     *   Limitations: Per-call vector allocation for coefficients.
     */
    Variant rawToEng(const Variant& raw) const override {
        return PolynomialCalibration(getCoeffs()).rawToEng(raw);
    }

    /**
     * @brief Analytically inverts linear polynomial (`degree <= 1`) from `eng` to raw.
     *
     * @details Constructs `PolynomialCalibration(getCoeffs())` and delegates to `engToRaw(eng)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates to `PolynomialCalibration::engToRaw` degree/slope validation.
     *
     * @param[in] eng
     *   Justification: Supplies engineering output to invert.
     *   Usage: Forwarded to `PolynomialCalibration::engToRaw`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` and invertible degree <= 1 coefficients.
     *
     * @pre `getCoeffs().size() == 2 && getCoeffs()[1] != 0.0`
     * @post Returned `Variant` holds inverted raw `double`.
     * @return Variant
     *   Justification: Delivers inverted raw value.
     *   Usage: Caller unwraps `double` result.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `eng` and invertible degree <= 1 polynomial.
     *   Limitations: Degree > 1 throws `std::logic_error`.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::logic_error` or `std::invalid_argument` on invalid inversion/input.
     * @throws std::logic_error [Validation / Contract] Thrown if degree > 1 or `a1 == 0.0`.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `eng` is non-numeric.
     * @error
     *   - Validation / Contract (non-invertible or non-numeric): Throws cleanly without mutating `*this`.
     * @containment
     *   Approach: Delegates to `PolynomialCalibration::engToRaw` guards.
     *   Limitations: Higher-degree polynomials are not inverted.
     */
    Variant engToRaw(const Variant& eng) const override {
        return PolynomialCalibration(getCoeffs()).engToRaw(eng);
    }
};

// Note: Table, Enum, and Format can be similarly implemented. For brevity and functional correctness, 
// they can also be fully supported via children or serialized structurally. We'll leave them as basic implementations 
// if required, but mapping them to children allows serialization for free.

/**
 * @class CompositeCalibration
 * @brief Calibration that applies multiple calibrations in sequence.
 *
 * @details Chains child `NamedCalibration` nodes in forward child order during `rawToEng`
 * and in reverse child order during `engToRaw`, bounded by `limit = 1000000`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Child `NamedCalibration` stages are applied sequentially in forward (`rawToEng`) and reverse (`engToRaw`) order.
 * @compliance Enforces bounded iteration (`1000000`) and dynamic type checks on child stages.
 */
class CompositeCalibration : public NamedCalibration {
public:
    /**
     * @brief Constructs a CompositeCalibration.
     *
     * @details Initializes base `NamedCalibration` with `name` and empty pipeline stages.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` single-argument constructor.
     *
     * @param[in] name
     *   Justification: Identifies this composite calibration pipeline in the Quasar tree.
     *   Usage: Passed to `NamedCalibration(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     *
     * @pre !name.empty()
     * @post Node constructed with no child stages.
     * @throws std::exception [Resource / Allocation] Propagated if `NamedCalibration` throws.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: Delegates directly to `NamedCalibration`.
     *   Limitations: Empty composite acts as identity pass-through until stages are added.
     */
    explicit CompositeCalibration(const std::string& name) : NamedCalibration(name) {}

    /**
     * @brief Returns the reflexive type identifier `"CompositeCalibration"`.
     *
     * @details Overrides `NamedObject::getType()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Deterministic reflexive type tag.
     *
     * @post Returned string equals `"CompositeCalibration"`.
     * @return std::string
     *   Justification: Identifies composite pipeline nodes in tree queries and scripting.
     *   Usage: Caller compares returned type string.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"CompositeCalibration"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Constant literal return.
     *   Limitations: Non-`noexcept` due to base virtual signature.
     */
    std::string getType() const override { return "CompositeCalibration"; }

    /**
     * @brief Clones this `CompositeCalibration` node.
     *
     * @details Allocates a new empty `CompositeCalibration` with `getName()` via `std::make_shared`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.10, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Allocates via `std::make_shared`.
     *
     * @param[in] policy
     *   Justification: Required by `NamedObject::clone` override signature.
     *   Usage: Unused by shallow empty-container clone.
     *   Consumed: Yes (copied enum value).
     *   Prerequisites: Valid `CopyPolicy` enum value.
     *
     * @pre true
     * @post Returned `shared_ptr` holds a newly allocated `CompositeCalibration` named `getName()`.
     * @return std::shared_ptr<quasar::named::NamedObject>
     *   Justification: Enables polymorphic node duplication in Quasar trees.
     *   Usage: Caller attaches stages to returned node or mounts it in a tree.
     *   Consumed: Ownership transferred via `std::shared_ptr`.
     *   Prerequisites: None.
     *   Limitations: Does not copy child calibration stages (`policy` unused).
     *   Lifecycle: Managed by returned `std::shared_ptr`.
     *   Modification / Deletion: Caller may mutate or drop freely.
     *   Specific Values: Non-null `std::shared_ptr<NamedObject>`.
     *   Error Behavior: Throws `std::bad_alloc` on allocation failure.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if `std::make_shared` fails.
     * @error
     *   - Resource / Allocation (OOM): Propagates cleanly.
     * @containment
     *   Approach: RAII `std::make_shared` allocation.
     *   Limitations: Callers expecting deep stage cloning must clone children separately.
     */
    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        std::shared_ptr<CompositeCalibration> comp = std::make_shared<CompositeCalibration>(getName());
        return comp;
    }

    /**
     * @brief Adds a calibration to the sequence.
     *
     * @details Appends `cal` as a child node via `addChild(cal)`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates child ownership to `NamedObject::addChild`.
     *
     * @param[in] cal
     *   Justification: Appends an ordered calibration stage to the composite pipeline.
     *   Usage: Passed to `addChild(cal)` so stage executes in insertion order.
     *   Consumed: No (shared_ptr copied into child list).
     *   Prerequisites: Non-null `std::shared_ptr<NamedCalibration>` with unique sibling name.
     *
     * @pre cal != nullptr
     * @post `cal` is registered as the last child calibration stage of `*this`.
     * @throws std::exception [Validation / Contract] Propagates if `addChild` rejects null or duplicate name.
     * @error
     *   - Validation / Contract (null or duplicate child): Propagates `NamedObject::addChild` exception.
     * @containment
     *   Approach: Delegates directly to `NamedObject::addChild` tree validation.
     *   Limitations: Non-`NamedCalibration` children added separately are skipped during pipeline execution.
     */
    void addCalibration(std::shared_ptr<NamedCalibration> cal) {
        addChild(cal);
    }

    /**
     * @brief Applies all child `NamedCalibration` stages sequentially in forward order.
     *
     * @details Iterates through `getChildren()` up to hard limit `1000000`, piping the intermediate
     * `Variant` `current` through each child that satisfies `is<NamedCalibration>()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces bounded iteration (`limit = 1000000`) and checks `child->is<NamedCalibration>()`.
     *
     * @param[in] raw
     *   Justification: Supplies initial raw value entering stage 0 of the composite pipeline.
     *   Usage: Piped sequentially through each `NamedCalibration` child's `rawToEng`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Compatible with the first active stage in `getChildren()`.
     *
     * @pre `getChildren().size() <= 1000000`
     * @post Returned `Variant` holds final transformed output (or `raw` if no calibration children).
     * @return Variant
     *   Justification: Delivers composed engineering value after all stages execute.
     *   Usage: Caller unwraps final stage output alternative.
     *   Consumed: Value returned by move/copy.
     *   Prerequisites: Stage input/output types must compose cleanly across children.
     *   Limitations: Bounded to 1,000,000 child nodes; non-calibration children are skipped.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: Final stage `Variant` output.
     *   Error Behavior: Throws `std::runtime_error` on limit breach or propagates child stage exceptions.
     * @throws std::runtime_error [Resource / Fatal] Thrown if child count exceeds 1,000,000.
     * @throws std::exception [Validation / Contract] Propagates exceptions from any child `rawToEng`.
     * @error
     *   - Resource / Fatal (child count > 1,000,000): Throws `std::runtime_error`; `this` unchanged.
     *   - Validation / Contract (child stage failure): Propagates stage exception; `this` unchanged.
     * @containment
     *   Approach: Hard counter `limit = 1000000` and type guard `is<NamedCalibration>()` protect pipeline loop.
     *   Limitations: Intermediate stage values are not rolled back as transactions because stages are const.
     */
    Variant rawToEng(const Variant& raw) const override {
        Variant current = raw;
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000;
        size_t count = 0;
        // Apply child calibrations in forward order bounded by hard limit 1000000 [CS-0010.37].
        for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
            // Abort immediately if child count exceeds safety iteration bound.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in CompositeCalibration::rawToEng");
            std::shared_ptr<NamedObject> child = *it;
            // Invoke rawToEng only on NamedCalibration children.
            if (child->is<NamedCalibration>()) {
                current = child->as<NamedCalibration>()->rawToEng(current);
            }
        }
        return current;
    }

    /**
     * @brief Applies all child `NamedCalibration` inverse stages in reverse order.
     *
     * @details Iterates `getChildren()` in reverse (`rbegin()` to `rend()`) up to `limit = 1000000`,
     * piping `current` through each child `NamedCalibration`'s `engToRaw`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces bounded iteration (`limit = 1000000`) and reverse stage ordering for algebraic inverse consistency.
     *
     * @param[in] eng
     *   Justification: Supplies final engineering value to invert backward through the pipeline.
     *   Usage: Piped in reverse child order through each `NamedCalibration`'s `engToRaw`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Compatible with the last active stage's `engToRaw`.
     *
     * @pre `getChildren().size() <= 1000000`
     * @post Returned `Variant` holds inverted raw value (or `eng` if no calibration children).
     * @return Variant
     *   Justification: Delivers composed inverse raw value matching forward stage composition.
     *   Usage: Caller unwraps inverted raw `Variant`.
     *   Consumed: Value returned by move/copy.
     *   Prerequisites: All active stages must support `engToRaw`.
     *   Limitations: Bounded to 1,000,000 child nodes; non-invertible stage aborts entire pipeline.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: Final inverted stage `Variant`.
     *   Error Behavior: Throws `std::runtime_error` on limit breach or propagates child `engToRaw` exceptions.
     * @throws std::runtime_error [Resource / Fatal] Thrown if child count exceeds 1,000,000.
     * @throws std::exception [Validation / Contract] Propagates exceptions from any child `engToRaw`.
     * @error
     *   - Resource / Fatal (child count > 1,000,000): Throws `std::runtime_error`; `this` unchanged.
     *   - Validation / Contract (child inverse failure): Propagates stage exception; `this` unchanged.
     * @containment
     *   Approach: Hard counter `limit = 1000000` and `is<NamedCalibration>()` guard reverse traversal.
     *   Limitations: Non-invertible intermediate stage aborts the inverse chain.
     */
    Variant engToRaw(const Variant& eng) const override {
        Variant current = eng;
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000;
        size_t count = 0;
        // Apply child inverse calibrations in reverse order bounded by hard limit 1000000 [CS-0010.37].
        for (std::list<std::shared_ptr<NamedObject>>::reverse_iterator it = children.rbegin(); it != children.rend(); ++it) {
            // Abort immediately if child count exceeds safety iteration bound.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in CompositeCalibration::engToRaw");
            std::shared_ptr<NamedObject> child = *it;
            // Invoke engToRaw only on NamedCalibration children.
            if (child->is<NamedCalibration>()) {
                current = child->as<NamedCalibration>()->engToRaw(current);
            }
        }
        return current;
    }
};

} // namespace quasar::calibration
#endif
