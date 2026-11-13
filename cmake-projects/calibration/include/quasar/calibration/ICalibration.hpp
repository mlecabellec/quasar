/**
 * @file ICalibration.hpp
 * @brief Declares the Variant value type and ICalibration bidirectional conversion interface.
 *
 * @details Defines the core polymorphism boundary for transforming raw hardware readings
 * into engineering units (`rawToEng`) and inverse engineering values back into raw units
 * (`engToRaw`) across all Quasar calibration components.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8
 * @compliance Enforces full Doxygen coverage on alias, interface class, destructor, and pure virtual methods.
 */

#ifndef QUASAR_CALIBRATION_ICALIBRATION_HPP
#define QUASAR_CALIBRATION_ICALIBRATION_HPP

// Standard variant, string, and fixed-width integer primitives.
#include <variant>
#include <string>
#include <cstdint>

/**
 * @brief Quasar engineering-unit calibration namespace.
 *
 * @details Houses calibration interfaces, mathematical/table/enum/format transforms,
 * hierarchical `NamedCalibration` wrappers, `CalibrationManager`, and Sol2 Lua bindings.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Encapsulates all calibration declarations inside `quasar::calibration`.
 */
namespace quasar::calibration {

/**
 * @typedef Variant
 * @brief Dynamic value type for data handling.
 *
 * @details Discriminated `std::variant` supporting empty state (`std::monostate`), booleans,
 * signed/unsigned 64-bit integers, IEEE-754 doubles, and UTF-8 strings for raw and engineering
 * unit conversions without `void*` or C-style unions.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0060.6, CS-0090.1, CS-0090.2
 * @exposed
 * @compliance Substitutes type-safe `std::variant` for raw unions per [CS-0060.6].
 */
using Variant = std::variant<std::monostate, bool, int64_t, uint64_t, double, std::string>;

/**
 * @interface ICalibration
 * @brief Abstract interface for bidirectional value transformation.
 *
 * @details Enables high-fidelity conversion between raw system data and engineering units
 * via `rawToEng` and `engToRaw`, preserving const-correctness and safe polymorphic teardown.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0060.5, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Implementations remain stateless or const-safe during `rawToEng` and `engToRaw` calls.
 * @compliance Defines pure virtual const interface and defaulted virtual destructor.
 */
class ICalibration {
public:
    /**
     * @brief Virtual destructor enabling safe polymorphic deletion of calibration instances.
     *
     * @details Defaulted noexcept virtual destructor ensuring derived calibration classes
     * release owned buffers cleanly through `ICalibration` pointers.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0060.5, CS-0090.1, CS-0090.2
     * @exposed
     * @compliance Defaulted virtual destructor guarantees exception-safe polymorphic cleanup.
     */
    virtual ~ICalibration() = default;

    /**
     * @brief Transforms a raw hardware value into an engineering value.
     *
     * @details Converts an input `Variant` representing raw ADC/bus units into the calibrated
     * engineering representation defined by the concrete implementation.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces const-qualified pure virtual contract with explicit error/return documentation.
     *
     * @param[in] raw
     *   Justification: Supplies the uncalibrated raw value read from hardware or protocol frames.
     *   Usage: Inspected read-only by concrete implementations to compute the calibrated output.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Must hold a variant alternative accepted by the concrete calibration (typically numeric or `int64_t`).
     *
     * @pre `raw` holds a type compatible with the concrete `ICalibration` implementation.
     * @post Returned `Variant` contains the calibrated engineering value; `this` is unmodified.
     * @return Variant
     *   Justification: Returns the transformed engineering value across heterogeneous scalar/string types.
     *   Usage: Caller unwraps expected alternative via `std::get` or `toDouble`/`toInt`.
     *   Consumed: Value returned by move/copy; caller owns returned `Variant`.
     *   Prerequisites: Caller must match expected alternative (`double`, `std::string`, etc.) before `std::get`.
     *   Limitations: Supported input/output alternatives depend on concrete subclass semantics.
     *   Lifecycle: Independent caller-owned `Variant` value.
     *   Modification / Deletion: Caller may mutate, move, or discard returned `Variant` freely.
     *   Specific Values: Numeric scalar, formatted string, or unchanged pass-through (identity).
     *   Error Behavior: Concrete implementations throw `std::invalid_argument`, `std::logic_error`, or `std::runtime_error` on failure.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `raw` holds an incompatible variant alternative or unknown enum key.
     * @throws std::runtime_error [Resource / Fatal] Thrown if bounded loop limits are breached in table/polynomial evaluation.
     * @error
     *   - Validation / Contract (incompatible `raw` type or out-of-domain key): Throws `std::invalid_argument`; strong guarantee (`this` const).
     *   - Resource / Fatal (iteration safety bound exceeded): Throws `std::runtime_error`; strong guarantee (`this` const).
     * @containment
     *   Approach: Const method signature prevents state mutation; concrete subclasses validate `std::variant` alternatives prior to computation.
     *   Limitations: Callers must catch `std::exception` when passing untrusted dynamic `Variant` inputs.
     */
    virtual Variant rawToEng(const Variant& raw) const = 0;

    /**
     * @brief Transforms an engineering value back into a raw hardware value.
     *
     * @details Applies the inverse calibration mapping to convert an engineering-unit `Variant`
     * back into raw hardware units when an analytical or lookup inverse exists.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces const-qualified pure virtual inverse contract with explicit error/return documentation.
     *
     * @param[in] eng
     *   Justification: Supplies the engineering-unit setpoint or value to convert back to raw units.
     *   Usage: Inspected read-only by concrete implementations to compute the inverse raw value.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Must hold a variant alternative accepted by the concrete inverse transform.
     *
     * @pre `eng` holds a type compatible with the concrete `ICalibration` inverse mapping.
     * @post Returned `Variant` contains the raw hardware value; `this` is unmodified.
     * @return Variant
     *   Justification: Supplies raw value suitable for writing back to hardware/bus registers.
     *   Usage: Caller unwraps alternative via `std::get`, `toDouble`, or `toInt`.
     *   Consumed: Value returned by move/copy; caller owns returned `Variant`.
     *   Prerequisites: Concrete calibration must support inversion (e.g., degree <= 1 for polynomials).
     *   Limitations: Non-invertible mappings (such as degree > 1 polynomials or constant terms) throw `std::logic_error`.
     *   Lifecycle: Independent caller-owned `Variant` value.
     *   Modification / Deletion: Caller may mutate, move, or discard returned `Variant` freely.
     *   Specific Values: Numeric scalar, integer enum key, or pass-through (identity).
     *   Error Behavior: Concrete implementations throw `std::invalid_argument`, `std::logic_error`, or `std::runtime_error` on failure.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `eng` has wrong type, unparseable string, or unknown enum label.
     * @throws std::logic_error [Validation / Contract] Thrown if the calibration is mathematically non-invertible.
     * @throws std::runtime_error [Resource / Fatal] Thrown if bounded loop limits are exceeded.
     * @error
     *   - Validation / Contract (non-invertible calibration or bad input type): Throws `std::logic_error` or `std::invalid_argument`; `this` untouched.
     *   - Resource / Fatal (iteration safety bound exceeded): Throws `std::runtime_error`.
     * @containment
     *   Approach: Const interface preserves object invariants and throws typed standard exceptions before returning corrupt raw data.
     *   Limitations: Callers must catch `std::exception` when inverting arbitrary calibration instances.
     */
    virtual Variant engToRaw(const Variant& eng) const = 0;
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_ICALIBRATION_HPP
