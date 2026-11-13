/**
 * @file Calibrations.hpp
 * @brief Concrete calibration implementations (Identity, Linear, Polynomial, Table, Enum, Format) and Variant helpers.
 *
 * @details Implements mathematical, lookup-table, enumeration, and string-formatting transforms
 * deriving from `ICalibration`, along with `toDouble` and `toInt` Variant extraction helpers.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Enforces complete Doxygen contracts on all classes, nested structs, fields,
 * constructors, helpers, and bounded loop control structures.
 */

#ifndef QUASAR_CALIBRATION_CALIBRATIONS_HPP
#define QUASAR_CALIBRATION_CALIBRATIONS_HPP

// Core calibration interface and standard math/container/formatting headers.
#include "ICalibration.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>
#include <map>
// Sorting, I/O manipulators, and string stream formatting utilities.
#include <algorithm>
#include <iomanip>
#include <sstream>

/**
 * @brief Quasar engineering-unit calibration namespace.
 *
 * @details Provides concrete stateless/immutable calibration algorithms implementing `ICalibration`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Scopes concrete calibration classes inside `quasar::calibration`.
 */
namespace quasar::calibration {

/**
 * @brief Extracts a double-precision floating-point value from a numeric `Variant`.
 *
 * @details Inspects `v` for `double`, `int64_t`, or `uint64_t` alternatives using
 * `std::holds_alternative`, converting integers explicitly via `static_cast<double>`, and
 * throwing `std::invalid_argument` for non-numeric alternatives.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0020.59, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Uses checked `std::holds_alternative` guards and explicit `static_cast<double>` conversions.
 *
 * @param[in] v
 *   Justification: Supplies the dynamic `Variant` to convert into a uniform IEEE-754 `double`.
 *   Usage: Queried via `std::holds_alternative` and `std::get` without modifying `v`.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Must hold `double`, `int64_t`, or `uint64_t`.
 *
 * @pre std::holds_alternative<double>(v) || std::holds_alternative<int64_t>(v) || std::holds_alternative<uint64_t>(v)
 * @post Returned `double` represents the numeric value of `v` (subject to 53-bit mantissa precision for 64-bit ints).
 * @return double
 *   Justification: Normalizes heterogeneous numeric `Variant` inputs for floating-point calibration formulas.
 *   Usage: Used directly as arithmetic operand in linear, polynomial, table, and format transforms.
 *   Consumed: Value returned by copy.
 *   Prerequisites: Input `v` must hold a supported numeric alternative.
 *   Limitations: `int64_t`/`uint64_t` magnitudes above 2^53 lose least-significant precision when cast to `double`.
 *   Lifecycle: Ephemeral scalar prvalue.
 *   Modification / Deletion: Immutable prvalue; no deletion required.
 *   Specific Values: Exact `double` value or widened `int64_t`/`uint64_t`.
 *   Error Behavior: Never returns sentinel; throws `std::invalid_argument` on non-numeric `Variant`.
 * @throws std::invalid_argument [Validation / Contract] Thrown if `v` holds `std::monostate`, `bool`, or `std::string`.
 * @error
 *   - Validation / Contract (non-numeric variant alternative): Throws `std::invalid_argument`; `v` remains untouched.
 * @containment
 *   Approach: Explicit `std::holds_alternative` chain validates type before `std::get`, preventing `std::bad_variant_access`.
 *   Limitations: Does not parse numeric strings or promote `bool` to `0.0`/`1.0`.
 */
inline double toDouble(const Variant& v) {
    // Return IEEE-754 double directly when already held.
    if (std::holds_alternative<double>(v)) return std::get<double>(v);
    // Explicitly widen signed 64-bit integer to double [CS-0020.59].
    if (std::holds_alternative<int64_t>(v)) return static_cast<double>(std::get<int64_t>(v));
    // Explicitly widen unsigned 64-bit integer to double [CS-0020.59].
    if (std::holds_alternative<uint64_t>(v)) return static_cast<double>(std::get<uint64_t>(v));
    throw std::invalid_argument("Variant does not hold a numeric type");
}

/**
 * @brief Extracts a signed 64-bit integer value from a numeric `Variant`.
 *
 * @details Inspects `v` for `int64_t`, `uint64_t`, or `double` alternatives, casting explicitly
 * to `int64_t`, and throws `std::invalid_argument` for non-numeric alternatives.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0020.59, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Uses `std::holds_alternative` checks and explicit `static_cast<int64_t>` narrowing/conversion.
 *
 * @param[in] v
 *   Justification: Supplies the dynamic `Variant` key for discrete integer lookups such as `EnumCalibration`.
 *   Usage: Inspected read-only via `std::holds_alternative` and `std::get`.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Must hold `int64_t`, `uint64_t`, or `double`.
 *
 * @pre std::holds_alternative<int64_t>(v) || std::holds_alternative<uint64_t>(v) || std::holds_alternative<double>(v)
 * @post Returned `int64_t` holds the extracted/truncated integer value of `v`.
 * @return int64_t
 *   Justification: Provides uniform signed 64-bit integer key for discrete enum calibration tables.
 *   Usage: Passed to `std::map<int64_t, std::string>::find`.
 *   Consumed: Value returned by copy.
 *   Prerequisites: Input `v` must hold a numeric alternative.
 *   Limitations: Truncates fractional `double` values and wraps `uint64_t` values > `INT64_MAX`.
 *   Lifecycle: Ephemeral scalar prvalue.
 *   Modification / Deletion: Immutable prvalue; no deletion required.
 *   Specific Values: Extracted `int64_t` integer.
 *   Error Behavior: Never returns sentinel; throws `std::invalid_argument` on non-numeric `Variant`.
 * @throws std::invalid_argument [Validation / Contract] Thrown if `v` holds `std::monostate`, `bool`, or `std::string`.
 * @error
 *   - Validation / Contract (non-numeric variant alternative): Throws `std::invalid_argument`; `v` untouched.
 * @containment
 *   Approach: Explicit `std::holds_alternative` checks guard `std::get` accesses cleanly.
 *   Limitations: Does not range-check `uint64_t` or `double` overflow before `static_cast<int64_t>`.
 */
inline int64_t toInt(const Variant& v) {
    // Return signed 64-bit integer directly when already held.
    if (std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v);
    // Explicitly convert unsigned 64-bit integer to int64_t [CS-0020.59].
    if (std::holds_alternative<uint64_t>(v)) return static_cast<int64_t>(std::get<uint64_t>(v));
    // Truncate IEEE-754 double explicitly to int64_t [CS-0020.59].
    if (std::holds_alternative<double>(v)) return static_cast<int64_t>(std::get<double>(v));
    throw std::invalid_argument("Variant does not hold an integer type");
}

/**
 * @class IdentityCalibration
 * @brief Pass-through calibration returning input variants unchanged in both directions.
 *
 * @details Implements `ICalibration` as an identity mapping (`rawToEng(x) == x`, `engToRaw(x) == x`)
 * for uncalibrated channels or composite pipeline placeholders.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0050.3, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Stateless identity transform preserving all `Variant` alternatives verbatim.
 * @compliance Follows Rule of Zero and implements `ICalibration` with const pass-through methods.
 */
class IdentityCalibration : public ICalibration {
public:
    /**
     * @brief Returns the raw input variant unmodified as the engineering value.
     *
     * @details Copies `raw` directly without type inspection or arithmetic.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Stateless identity pass-through preserving `Variant` value and type.
     *
     * @param[in] raw
     *   Justification: Supplies input `Variant` to pass through unchanged.
     *   Usage: Copied directly into return value.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Any initialized `Variant`.
     *
     * @pre true
     * @post Returned `Variant` equals `raw`.
     * @return Variant
     *   Justification: Satisfies `ICalibration::rawToEng` contract for uncalibrated channels.
     *   Usage: Caller consumes returned copy directly.
     *   Consumed: Value returned by copy; caller owns result.
     *   Prerequisites: None.
     *   Limitations: Performs no numeric validation.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: Identical to `raw`.
     *   Error Behavior: Propagates `std::bad_alloc` only if `raw` holds a long `std::string` and copy allocation fails.
     * @throws std::bad_alloc [Resource / Allocation] Possible if copying a `std::string` payload fails.
     * @error
     *   - Resource / Allocation (string copy OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Zero state mutation; direct copy return.
     *   Limitations: Does not reject `std::monostate` inputs.
     */
    Variant rawToEng(const Variant& raw) const override { return raw; }

    /**
     * @brief Returns the engineering input variant unmodified as the raw value.
     *
     * @details Copies `eng` directly without type inspection or arithmetic.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Stateless identity pass-through preserving `Variant` value and type.
     *
     * @param[in] eng
     *   Justification: Supplies input `Variant` to pass through unchanged.
     *   Usage: Copied directly into return value.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Any initialized `Variant`.
     *
     * @pre true
     * @post Returned `Variant` equals `eng`.
     * @return Variant
     *   Justification: Satisfies `ICalibration::engToRaw` contract for uncalibrated channels.
     *   Usage: Caller consumes returned copy directly.
     *   Consumed: Value returned by copy; caller owns result.
     *   Prerequisites: None.
     *   Limitations: Performs no numeric validation.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: Identical to `eng`.
     *   Error Behavior: Propagates `std::bad_alloc` only if `eng` holds a `std::string` and copy fails.
     * @throws std::bad_alloc [Resource / Allocation] Possible if copying a `std::string` payload fails.
     * @error
     *   - Resource / Allocation (string copy OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Zero state mutation; direct copy return.
     *   Limitations: Does not reject `std::monostate` inputs.
     */
    Variant engToRaw(const Variant& eng) const override { return eng; }
};

/**
 * @class LinearCalibration
 * @brief Affine linear transformation (`y = x * scale + offset`) and its exact inverse.
 *
 * @details Validates `scale != 0.0` at construction so `engToRaw` division by `m_scale`
 * is always well-defined.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0010.19, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `m_scale != 0.0`.
 * @compliance Enforces non-zero scale invariant in constructor to prevent division by zero in `engToRaw`.
 */
class LinearCalibration : public ICalibration {
public:
    /**
     * @brief Constructs a LinearCalibration with non-zero scale and offset.
     *
     * @details Stores `scale` and `offset`, throwing `std::invalid_argument` immediately if
     * `m_scale == 0.0`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.19, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Rejects zero scale factor at construction to uphold invertibility invariant.
     *
     * @param[in] scale
     *   Justification: Defines the linear slope multiplier (`eng = raw * scale + offset`).
     *   Usage: Stored in `m_scale` and validated against `0.0`.
     *   Consumed: Yes (copied scalar `double`).
     *   Prerequisites: Finite non-zero `double` (`scale != 0.0`).
     * @param[in] offset
     *   Justification: Defines the constant additive y-intercept.
     *   Usage: Stored in `m_offset` and added/subtracted during transformations.
     *   Consumed: Yes (copied scalar `double`).
     *   Prerequisites: Finite `double` value.
     *
     * @pre scale != 0.0
     * @post `m_scale == scale && m_offset == offset`.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `scale == 0.0`.
     * @error
     *   - Validation / Contract (`scale == 0.0`): Throws `std::invalid_argument` preventing invalid instance creation.
     * @containment
     *   Approach: Eager constructor validation prevents creating a non-invertible `LinearCalibration`.
     *   Limitations: Does not reject `NaN` or `Inf` floating-point inputs.
     */
    LinearCalibration(double scale, double offset) : m_scale(scale), m_offset(offset) {
        // Reject zero slope at construction so engToRaw never divides by zero [CS-0010.19].
        if (m_scale == 0.0) throw std::invalid_argument("Scale cannot be zero");
    }

    /**
     * @brief Transforms raw numeric value using `x * m_scale + m_offset`.
     *
     * @details Extracts `double` via `toDouble(raw)` and returns the resulting `double` `Variant`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Uses `toDouble` type guard before floating-point evaluation.
     *
     * @param[in] raw
     *   Justification: Supplies raw numeric input to scale and shift.
     *   Usage: Converted via `toDouble(raw)` and transformed linearly.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `raw` holds a numeric variant alternative.
     * @post Returned `Variant` holds `double` equal to `toDouble(raw) * m_scale + m_offset`.
     * @return Variant
     *   Justification: Returns calibrated engineering value in `double` representation.
     *   Usage: Caller retrieves `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `raw` input.
     *   Limitations: Result is always `double` even if input was `int64_t`.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` if `raw` is non-numeric.
     * @throws std::invalid_argument [Validation / Contract] Propagated from `toDouble(raw)`.
     * @error
     *   - Validation / Contract (non-numeric `raw`): Throws `std::invalid_argument`; object state unchanged.
     * @containment
     *   Approach: `toDouble(raw)` validates variant type before arithmetic.
     *   Limitations: Floating-point overflow/underflow follows IEEE-754 semantics.
     */
    Variant rawToEng(const Variant& raw) const override {
        // Convert input to double and initialize polynomial evaluation terms and loop bound.
        double x = toDouble(raw);
        return x * m_scale + m_offset;
    }

    /**
     * @brief Transforms engineering numeric value back to raw using `(y - m_offset) / m_scale`.
     *
     * @details Extracts `double` via `toDouble(eng)` and divides by guaranteed non-zero `m_scale`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Relies on constructor invariant `m_scale != 0.0` and `toDouble` type guard.
     *
     * @param[in] eng
     *   Justification: Supplies engineering-unit value to invert back to raw units.
     *   Usage: Converted via `toDouble(eng)` and inverted linearly.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `eng` holds a numeric variant alternative.
     * @post Returned `Variant` holds `double` equal to `(toDouble(eng) - m_offset) / m_scale`.
     * @return Variant
     *   Justification: Returns inverted raw value as `double`.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `eng` input.
     *   Limitations: Subject to IEEE-754 rounding when dividing by `m_scale`.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` if `eng` is non-numeric.
     * @throws std::invalid_argument [Validation / Contract] Propagated from `toDouble(eng)`.
     * @error
     *   - Validation / Contract (non-numeric `eng`): Throws `std::invalid_argument`; object state unchanged.
     * @containment
     *   Approach: Constructor guarantees `m_scale != 0.0` and `toDouble` validates input type.
     *   Limitations: IEEE-754 precision loss may leave small residual error relative to original integer raw input.
     */
    Variant engToRaw(const Variant& eng) const override {
        double y = toDouble(eng);
        return (y - m_offset) / m_scale;
    }
private:
    /**
     * @brief Non-zero linear slope multiplier applied in `rawToEng` and divided in `engToRaw`.
     * @details Enforced `!= 0.0` by `LinearCalibration` constructor.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Immutable private scalar initialized at construction.
     */
    double m_scale;

    /**
     * @brief Constant additive offset applied in `rawToEng` and subtracted in `engToRaw`.
     * @details Initialized at construction.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Immutable private scalar initialized at construction.
     */
    double m_offset;
};

/**
 * @class PolynomialCalibration
 * @brief Evaluates ascending-power polynomials `sum(a_i * x^i)` with bounded iteration.
 *
 * @details Supports arbitrary polynomial evaluation in `rawToEng` up to 1000 coefficients and
 * analytical inversion in `engToRaw` for constant/linear polynomials (degree <= 1).
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170
 * @constraint CS-0010.2, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `m_coeffs[i]` stores the coefficient of `x^i`.
 * @compliance Uses move semantics in constructor and hard loop bound `1000` in `rawToEng`.
 */
class PolynomialCalibration : public ICalibration {
public:
    /**
     * @brief Constructs a PolynomialCalibration from ascending-order coefficients.
     *
     * @details Moves `coeffs` into `m_coeffs` per [CS-0010.2].
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.2, CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` constructor transferring `coeffs` via `std::move`.
     *
     * @param[in] coeffs
     *   Justification: Supplies ascending polynomial coefficients `[a0, a1, ..., an]`.
     *   Usage: Moved into `m_coeffs` storage.
     *   Consumed: Yes (moved-from via `std::move` when caller supplies rvalue/value).
     *   Prerequisites: Vector of finite coefficients (ideally `coeffs.size() <= 1000`).
     *
     * @pre coeffs.size() <= 1000 for guaranteed bounded `rawToEng` execution
     * @post `m_coeffs` contains the supplied coefficients.
     * @error
     *   - Validation / Contract (more than 1000 coefficients): Accepted at construction; rejected at `rawToEng` runtime.
     * @containment
     *   Approach: Single move assignment into `m_coeffs` with noexcept vector move.
     *   Limitations: Does not eagerly reject `coeffs.size() > 1000` in constructor.
     */
    explicit PolynomialCalibration(std::vector<double> coeffs) : m_coeffs(std::move(coeffs)) {}

    /**
     * @brief Transforms raw value to engineering value using polynomial coefficients.
     *
     * @details Evaluates `sum(m_coeffs[i] * x^i)` iteratively with explicit loop counter `count`
     * bounded by `limit = 1000` per [CS-0010.37].
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces hard iteration bound `1000` and throws `std::runtime_error` on breach.
     *
     * @param[in] raw
     *   Justification: Supplies the raw numeric input `x` at which to evaluate the polynomial.
     *   Usage: Converted via `toDouble(raw)` and multiplied across successive powers of `x`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `raw` holds a numeric variant alternative and `m_coeffs.size() <= 1000`.
     * @post Returned `Variant` holds `double` equal to polynomial sum at `toDouble(raw)`.
     * @return Variant
     *   Justification: Provides evaluated engineering scalar in `double` format.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `raw` input.
     *   Limitations: Bounded to 1000 coefficients; large `x` with high degree may overflow to `Inf`.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)` (`0.0` if `m_coeffs` is empty).
     *   Error Behavior: Throws `std::invalid_argument` on bad input or `std::runtime_error` on >1000 coefficients.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `raw` is not numeric.
     * @throws std::runtime_error [Resource / Fatal] If loop limit exceeded (>1000 coefficients).
     * @error
     *   - Validation / Contract (non-numeric `raw`): Throws `std::invalid_argument`; `this` unchanged.
     *   - Resource / Fatal (coefficient count > 1000): Throws `std::runtime_error`; `this` unchanged.
     * @containment
     *   Approach: Explicit iteration counter `limit = 1000` bounds loop execution time deterministically.
     *   Limitations: Uses ascending power multiplication (`x_pow *= x`) rather than Horner's method.
     */
    Variant rawToEng(const Variant& raw) const override {
        // Convert input to double and initialize polynomial evaluation terms and loop bound.
        double x = toDouble(raw);
        double result = 0.0;
        double x_pow = 1.0;
        const size_t limit = 1000; // Safety limit [CS-0010.37]
        size_t count = 0;
        // Iterate over ascending coefficients with explicit hard bound `limit` [CS-0010.37].
        for (std::vector<double>::const_iterator it = m_coeffs.begin(); it != m_coeffs.end(); ++it) {
            // Throw immediately if coefficient count exceeds hard safety limit.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in PolynomialCalibration::rawToEng");
            double coeff = *it;
            result += coeff * x_pow;
            x_pow *= x;
        }
        return result;
    }

    /**
     * @brief Transforms engineering value back to raw value. Only supported for degree <= 1.
     *
     * @details Analytically inverts `y = a0 + a1 * x` when `m_coeffs.size() <= 2` and `a1 != 0.0`;
     * throws `std::logic_error` for non-invertible or higher-degree polynomials.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.19, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards degree > 1 and zero linear slope before division.
     *
     * @param[in] eng
     *   Justification: Supplies engineering output `y` to analytically invert to raw `x`.
     *   Usage: Converted via `toDouble(eng)` when degree <= 1 and `a1 != 0.0`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` when polynomial is linear and invertible.
     *
     * @pre `m_coeffs.size() <= 2 && m_coeffs.size() > 1 && m_coeffs[1] != 0.0`
     * @post Returned `Variant` holds `(toDouble(eng) - a0) / a1`.
     * @return Variant
     *   Justification: Returns exact analytical inverse for linear polynomials.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Linear invertible polynomial (`m_coeffs.size() == 2 && a1 != 0.0`).
     *   Limitations: Degree >= 2 polynomials are not inverted numerically and throw `std::logic_error`.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::logic_error` if non-invertible/degree>1, or `std::invalid_argument` if `eng` non-numeric.
     * @throws std::logic_error [Validation / Contract] Thrown if `a1 == 0.0` or `m_coeffs.size() > 2`.
     * @throws std::invalid_argument [Validation / Contract] Thrown by `toDouble(eng)` on non-numeric input.
     * @error
     *   - Validation / Contract (degree > 1 or `a1 == 0.0`): Throws `std::logic_error`; `this` untouched.
     *   - Validation / Contract (non-numeric `eng`): Throws `std::invalid_argument`; `this` untouched.
     * @containment
     *   Approach: Rejects degree > 1 and zero slope prior to division, avoiding division-by-zero or ambiguous roots.
     *   Limitations: Does not attempt iterative root-finding for quadratic/cubic polynomials.
     */
    Variant engToRaw(const Variant& eng) const override {
        // Check whether polynomial is at most linear (degree <= 1).
        if (m_coeffs.size() <= 2) {
            double a0 = m_coeffs.empty() ? 0.0 : m_coeffs[0];
            double a1 = m_coeffs.size() > 1 ? m_coeffs[1] : 0.0;
            // Reject zero linear coefficient because constant polynomials are non-invertible [CS-0010.19].
            if (a1 == 0.0) throw std::logic_error("Non-invertible polynomial");
            double y = toDouble(eng);
            return (y - a0) / a1;
        }
        throw std::logic_error("engToRaw for polynomial degree > 1 is not supported analytically");
    }
private:
    /**
     * @brief Polynomial coefficients ordered from constant term `a0` upward.
     * @details Populated by move in constructor and read immutably in `rawToEng`/`engToRaw`.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Private vector encapsulated inside `PolynomialCalibration`.
     */
    std::vector<double> m_coeffs;
};

/**
 * @class TableCalibration
 * @brief Calibration using a lookup table with linear interpolation.
 *
 * @details Sorts breakpoints ascending by `x` on construction, clamps out-of-range inputs
 * to boundary endpoints, and performs piecewise linear interpolation in both `rawToEng`
 * and `engToRaw`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170
 * @constraint CS-0010.2, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `!m_points.empty()` and `m_points` is sorted in ascending `x` order.
 * @compliance Enforces non-empty table invariant at construction and bounded loop search in `interpolate`.
 */
class TableCalibration : public ICalibration {
public:
    /**
     * @struct Point
     * @brief Point in the lookup table pairing raw `x` with engineering `y`.
     * @details Value-type coordinate pair sorted by `x` during `TableCalibration` construction.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2
     * @exposed
     * @compliance Plain aggregate storing explicit `double` coordinates.
     */
    struct Point {
        /**
         * @brief Raw domain coordinate (`x`) of the lookup table breakpoint.
         * @details Used as the sorting key and independent variable in `rawToEng`.
         * @task TSK-20261113-003, TSK-20260311-010
         * @feature FE-0050, FE-0160
         * @constraint CS-0090.1, CS-0090.2
         * @compliance Explicit `double` member field.
         */
        double x;

        /**
         * @brief Engineering range coordinate (`y`) corresponding to raw `x`.
         * @details Used as interpolated output in `rawToEng` and search key in `engToRaw`.
         * @task TSK-20261113-003, TSK-20260311-010
         * @feature FE-0050, FE-0160
         * @constraint CS-0090.1, CS-0090.2
         * @compliance Explicit `double` member field.
         */
        double y;
    };

    /**
     * @brief Constructs a TableCalibration.
     *
     * @details Moves `points` into `m_points`, validates `!m_points.empty()`, and sorts
     * breakpoints ascending by `Point::x`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.2, CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Rejects empty point sets and maintains ascending `x` order invariant.
     *
     * @param[in] points
     *   Justification: Defines piecewise linear `(x, y)` calibration breakpoints.
     *   Usage: Moved into `m_points`, checked non-empty, and sorted ascending by `x`.
     *   Consumed: Yes (moved-from via `std::move`).
     *   Prerequisites: Non-empty vector (`!points.empty()`) with finite coordinates.
     *
     * @pre !points.empty()
     * @post `!m_points.empty()` and `m_points` is sorted ascending by `x`.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `points` is empty.
     * @error
     *   - Validation / Contract (empty `points`): Throws `std::invalid_argument` before construction completes.
     * @containment
     *   Approach: Empty guard prevents out-of-bounds indexing in `interpolate()`.
     *   Limitations: Duplicate `x` values produce flat vertical segments where `rawToEng` returns `y0`.
     */
    explicit TableCalibration(std::vector<Point> points) : m_points(std::move(points)) {
        // Enforce non-empty table invariant before sorting or interpolation.
        if (m_points.empty()) throw std::invalid_argument("Point pairs cannot be empty");
        std::sort(m_points.begin(), m_points.end(), [](const Point& a, const Point& b) {
            return a.x < b.x;
        });
    }

    /**
     * @brief Interpolates engineering value `y` from raw input `raw`.
     *
     * @details Converts `raw` via `toDouble(raw)` and delegates forward piecewise interpolation
     * (`isRawToEng = true`) to `interpolate()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates to bounded `interpolate()` with validated numeric extraction.
     *
     * @param[in] raw
     *   Justification: Supplies raw domain value `x` to look up or interpolate.
     *   Usage: Converted via `toDouble(raw)` and passed to `interpolate(..., true)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `raw` holds a numeric variant alternative.
     * @post Returned `Variant` holds clamped/interpolated `double` engineering value.
     * @return Variant
     *   Justification: Provides piecewise-linearly interpolated engineering output.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `raw` input.
     *   Limitations: Extrapolation clamps to boundary `y` values rather than extending linearly.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)` clamped to `[min_y, max_y]`.
     *   Error Behavior: Throws `std::invalid_argument` on non-numeric `raw` or `std::runtime_error` if table >1e6 segments.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `raw` is non-numeric.
     * @throws std::runtime_error [Resource / Fatal] Thrown if segment count exceeds 1,000,000.
     * @error
     *   - Validation / Contract (non-numeric `raw`): Throws `std::invalid_argument`.
     *   - Resource / Fatal (table size > 1,000,000): Throws `std::runtime_error`.
     * @containment
     *   Approach: Delegates to `interpolate()` enforcing hard segment loop bound and endpoint clamping.
     *   Limitations: Linear O(N) segment scan rather than binary search.
     */
    Variant rawToEng(const Variant& raw) const override {
        return interpolate(toDouble(raw), true);
    }

    /**
     * @brief Interpolates raw value `x` from engineering input `eng`.
     *
     * @details Converts `eng` via `toDouble(eng)` and delegates inverse piecewise interpolation
     * (`isRawToEng = false`) to `interpolate()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Delegates to bounded `interpolate()` assuming monotonic `y` progression.
     *
     * @param[in] eng
     *   Justification: Supplies engineering value `y` to invert back to raw domain `x`.
     *   Usage: Converted via `toDouble(eng)` and passed to `interpolate(..., false)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `eng` holds a numeric variant alternative.
     * @post Returned `Variant` holds clamped/interpolated `double` raw value.
     * @return Variant
     *   Justification: Provides piecewise-linearly inverted raw output.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Numeric `eng` input; table `y` values should be monotonic for unique inverse.
     *   Limitations: Non-monotonic `y` curves return the first matching segment; clamps out-of-range inputs.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` on non-numeric `eng` or `std::runtime_error` on >1e6 segments.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `eng` is non-numeric.
     * @throws std::runtime_error [Resource / Fatal] Thrown if segment count exceeds 1,000,000.
     * @error
     *   - Validation / Contract (non-numeric `eng`): Throws `std::invalid_argument`.
     *   - Resource / Fatal (table size > 1,000,000): Throws `std::runtime_error`.
     * @containment
     *   Approach: Delegates to `interpolate()` enforcing hard segment loop bound and flat-segment divide-by-zero guard.
     *   Limitations: Inverse clamping uses endpoint `y` values rather than global min/max across non-monotonic tables.
     */
    Variant engToRaw(const Variant& eng) const override {
        return interpolate(toDouble(eng), false);
    }

private:
    /**
     * @brief Sorted vector of `(x, y)` calibration breakpoints.
     * @details Non-empty invariant guaranteed by constructor.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Private sorted storage owned by `TableCalibration`.
     */
    std::vector<Point> m_points;

    /**
     * @brief Interpolates values within the table.
     *
     * @details Handles single-point tables, clamps values outside endpoint bounds, and scans
     * adjacent point pairs up to `limit = 1000000` to compute linear interpolation while
     * guarding flat segments (`x1 == x0` or `y1 == y0`).
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.19, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @compliance Enforces bounded iteration (`limit = 1000000`) and zero-denominator guards.
     *
     * @param[in] val
     *   Justification: Supplies the scalar coordinate (`x` or `y`) to interpolate.
     *   Usage: Clamped against table bounds and interpolated within matching segment.
     *   Consumed: Yes (copied scalar `double`).
     *   Prerequisites: Finite `double` value.
     * @param[in] isRawToEng
     *   Justification: Selects forward (`x -> y`) vs inverse (`y -> x`) interpolation direction.
     *   Usage: Branches clamping and segment ratio calculation.
     *   Consumed: Yes (copied scalar `bool`).
     *   Prerequisites: None (`true` for `rawToEng`, `false` for `engToRaw`).
     *
     * @pre !m_points.empty() && m_points.size() <= 1000001
     * @post Returned `double` lies within table coordinate bounds (unless input is `NaN`).
     * @return double
     *   Justification: Supplies interpolated scalar to `rawToEng` or `engToRaw`.
     *   Usage: Wrapped into `Variant` and returned to caller.
     *   Consumed: Value returned by copy.
     *   Prerequisites: Non-empty `m_points`.
     *   Limitations: Linear scan complexity O(N); assumes monotonic `y` when `isRawToEng == false`.
     *   Lifecycle: Ephemeral scalar prvalue.
     *   Modification / Deletion: Immutable prvalue; no cleanup needed.
     *   Specific Values: Clamped endpoint or linearly interpolated `double`.
     *   Error Behavior: Throws `std::runtime_error` if `count > 1000000`.
     * @throws std::runtime_error [Resource / Fatal] If loop limit exceeded (>1,000,000 segments).
     * @error
     *   - Resource / Fatal (segment count > 1,000,000): Throws `std::runtime_error`; `this` unchanged.
     * @containment
     *   Approach: Checks single-point and endpoint clamping first, protects `x1 == x0` / `y1 == y0` from division by zero, and bounds loop with `limit = 1000000`.
     *   Limitations: `NaN` inputs bypass clamping comparisons and fall through to last endpoint.
     */
    double interpolate(double val, bool isRawToEng) const {
        // Clamp to endpoints immediately if table has only one point.
        if (m_points.size() == 1) {
            return isRawToEng ? m_points[0].y : m_points[0].x;
        }
        
        // Clamp out-of-range inputs according to forward or inverse interpolation direction.
        if (isRawToEng) {
            // Clamp values below minimum x or above maximum x.
            if (val <= m_points.front().x) return m_points.front().y;
            // Clamp values at or above highest raw breakpoint.
            if (val >= m_points.back().x) return m_points.back().y;
        } else {
            // engToRaw assumes monotonic y values. We handle mostly increasing bounds here.
            double min_y = m_points.front().y;
            double max_y = m_points.back().y;
            // Normalize endpoint bounds when table engineering values decrease monotonically.
            if (min_y > max_y) std::swap(min_y, max_y);
            // Clamp engineering values at or below minimum boundary.
            if (val <= min_y) return (m_points.front().y == min_y) ? m_points.front().x : m_points.back().x;
            // Clamp engineering values at or above maximum boundary.
            if (val >= max_y) return (m_points.back().y == max_y) ? m_points.back().x : m_points.front().x;
        }

        const size_t limit = 1000000; // Safety limit [CS-0010.37]
        size_t count = 0;
        // Scan adjacent point pairs bounded by m_points.size() - 1 and hard safety limit [CS-0010.37].
        for (size_t i = 0; i < m_points.size() - 1; ++i) {
            // Throw immediately if table segment count breaches hard safety limit.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in TableCalibration::interpolate");
            double x0 = m_points[i].x;
            double y0 = m_points[i].y;
            double x1 = m_points[i+1].x;
            double y1 = m_points[i+1].y;

            // Interpolate along x->y in forward mode or y->x in reverse mode.
            if (isRawToEng) {
                // Check if raw val falls inside current [x0, x1] segment.
                if (val >= x0 && val <= x1) {
                    // Guard against zero x-span division on duplicate x breakpoints.
                    if (x1 == x0) return y0;
                    return y0 + (y1 - y0) * (val - x0) / (x1 - x0);
                }
            } else {
                // Compute segment y interval regardless of ascending/descending slope.
                double min_val_y = std::min(y0, y1);
                double max_val_y = std::max(y0, y1);
                // Check if engineering val falls inside current [min_val_y, max_val_y] segment.
                if (val >= min_val_y && val <= max_val_y) {
                    // Guard against zero y-span division on flat segments.
                    if (y1 == y0) return x0;
                    return x0 + (x1 - x0) * (val - y0) / (y1 - y0);
                }
            }
        }
        // Fallback return if floating-point edge/NaN bypassed segment intervals.
        return isRawToEng ? m_points.back().y : m_points.back().x; 
    }
};

/**
 * @class EnumCalibration
 * @brief Calibration using a bidirectional discrete integer-to-string key-value mapping.
 *
 * @details Pre-builds a reverse lookup map (`std::string -> int64_t`) at construction with a
 * bounded loop limit, enabling O(log N) lookups in both `rawToEng` and `engToRaw`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170
 * @constraint CS-0010.2, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `m_map` and `m_reverse_map` stay synchronized after construction.
 * @compliance Uses bounded construction loop (`limit = 1000000`) and validates lookups before returning.
 */
class EnumCalibration : public ICalibration {
public:
    /**
     * @brief Constructs an EnumCalibration.
     *
     * @details Moves `mapping` into `m_map` and populates `m_reverse_map` under a hard
     * iteration bound of `1000000` entries.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.2, CS-0010.31, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Moves mapping efficiently and enforces hard bound `1000000` on reverse map construction.
     *
     * @param[in] mapping
     *   Justification: Defines discrete integer-to-label enumeration pairs.
     *   Usage: Moved into `m_map` and traversed to build `m_reverse_map`.
     *   Consumed: Yes (moved-from via `std::move`).
     *   Prerequisites: Map containing at most `1000000` unique entries.
     *
     * @pre mapping.size() <= 1000000
     * @post Both `m_map` and `m_reverse_map` are populated consistently.
     * @throws std::runtime_error [Resource / Fatal] Thrown if `mapping` exceeds 1,000,000 entries.
     * @throws std::bad_alloc [Resource / Allocation] Thrown if reverse map insertion exhausts heap memory.
     * @error
     *   - Resource / Fatal (mapping size > 1,000,000): Throws `std::runtime_error` cleanly.
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Hard loop bound `limit = 1000000` prevents unbounded map indexing.
     *   Limitations: Duplicate string labels in `mapping` overwrite earlier keys in `m_reverse_map`.
     */
    explicit EnumCalibration(std::map<int64_t, std::string> mapping) : m_map(std::move(mapping)) {
        const size_t limit = 1000000;
        size_t count = 0;
        // Build reverse string->int64 lookup map bounded by limit [CS-0010.37].
        for (std::map<int64_t, std::string>::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
            // Abort construction if mapping exceeds hard safety limit.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in EnumCalibration constructor");
            const int64_t& k = it->first;
            const std::string& v = it->second;
            m_reverse_map[v] = k;
        }
    }

    /**
     * @brief Maps an integer raw value to its configured enumeration string label.
     *
     * @details Converts `raw` via `toInt(raw)`, looks up `k` in `m_map`, and throws
     * `std::invalid_argument` if the key is absent.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Rejects non-numeric variants and unmapped integer keys with `std::invalid_argument`.
     *
     * @param[in] raw
     *   Justification: Supplies discrete raw integer key to resolve to text label.
     *   Usage: Converted via `toInt(raw)` and looked up in `m_map`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` whose integer value exists in `m_map`.
     *
     * @pre `raw` holds a numeric variant whose `toInt(raw)` key is in `m_map`.
     * @post Returned `Variant` holds `std::string` matching `m_map.at(toInt(raw))`.
     * @return Variant
     *   Justification: Returns human-readable enumeration label for raw integer state.
     *   Usage: Caller unwraps `std::string` via `std::get<std::string>`.
     *   Consumed: Value returned by copy; caller owns returned `std::string` variant.
     *   Prerequisites: Key must exist in `m_map`.
     *   Limitations: Only exact integer keys match.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(std::string)` from `m_map`.
     *   Error Behavior: Throws `std::invalid_argument` on non-numeric input or unknown key.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `raw` is non-numeric or key is unknown.
     * @error
     *   - Validation / Contract (non-numeric `raw` or unknown key): Throws `std::invalid_argument`; `this` unchanged.
     * @containment
     *   Approach: Validates type via `toInt(raw)` and checks `it != m_map.end()` before returning.
     *   Limitations: Does not provide a fallback default string for unknown enum codes.
     */
    Variant rawToEng(const Variant& raw) const override {
        int64_t k = toInt(raw);
        std::map<int64_t, std::string>::const_iterator it = m_map.find(k);
        // Return mapped string label when integer key exists in m_map.
        if (it != m_map.end()) return it->second;
        throw std::invalid_argument("Unknown enum key");
    }

    /**
     * @brief Maps an enumeration string label back to its raw `int64_t` key.
     *
     * @details Verifies `std::holds_alternative<std::string>(eng)`, queries `m_reverse_map`,
     * and throws `std::invalid_argument` on type mismatch or unknown label.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards variant string alternative and reverse-map lookup before returning key.
     *
     * @param[in] eng
     *   Justification: Supplies the engineering enumeration string label to invert.
     *   Usage: Inspected as `std::string` and looked up in `m_reverse_map`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: `Variant` holding `std::string` present in `m_reverse_map`.
     *
     * @pre std::holds_alternative<std::string>(eng)
     * @post Returned `Variant` holds `int64_t` matching `m_reverse_map.at(std::get<std::string>(eng))`.
     * @return Variant
     *   Justification: Supplies raw integer code corresponding to enumeration string.
     *   Usage: Caller unwraps `int64_t` via `std::get<int64_t>` or `toInt`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: `eng` holds a valid mapped `std::string`.
     *   Limitations: Case-sensitive exact string matching only.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(int64_t)`.
     *   Error Behavior: Throws `std::invalid_argument` on non-string input or unknown label.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `eng` is not `std::string` or label is unknown.
     * @error
     *   - Validation / Contract (non-string `eng` or unknown label): Throws `std::invalid_argument`; `this` untouched.
     * @containment
     *   Approach: Explicit `std::holds_alternative<std::string>` check prevents `std::bad_variant_access`.
     *   Limitations: Ambiguous duplicate labels resolve to the last key inserted during construction.
     */
    Variant engToRaw(const Variant& eng) const override {
        // Require string alternative before accessing enumeration label.
        if (!std::holds_alternative<std::string>(eng)) throw std::invalid_argument("Expected string for enum");
        const std::string& sv = std::get<std::string>(eng);
        std::map<std::string, int64_t>::const_iterator it = m_reverse_map.find(sv);
        // Return corresponding integer code when string label exists in reverse map.
        if (it != m_reverse_map.end()) return it->second;
        throw std::invalid_argument("Unknown enum string");
    }
private:
    /**
     * @brief Forward map from raw integer codes to engineering string labels.
     * @details Owned by `EnumCalibration` and populated in constructor.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Encapsulated private lookup map.
     */
    std::map<int64_t, std::string> m_map;

    /**
     * @brief Reverse map from engineering string labels to raw integer codes.
     * @details Built deterministically from `m_map` in constructor.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Encapsulated private reverse lookup map.
     */
    std::map<std::string, int64_t> m_reverse_map;
};

/**
 * @class FormatCalibration
 * @brief Formats numeric raw values into fixed-precision strings with an optional unit suffix.
 *
 * @details Renders `double` inputs with `std::fixed` precision `m_precision` and optional
 * `" <m_suffix>"` in `rawToEng`, and strips the suffix before parsing via `std::stod` in
 * `engToRaw`.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170
 * @constraint CS-0010.2, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant `m_precision` configures fixed decimal digits and `m_suffix` holds optional unit label.
 * @compliance Moves suffix parameter cleanly at construction and validates variant types on both paths.
 */
class FormatCalibration : public ICalibration {
public:
    /**
     * @brief Constructs a FormatCalibration with decimal precision and unit suffix.
     *
     * @details Stores `precision` and moves `suffix` into `m_suffix`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0010.2, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Transfers string suffix via `std::move` per [CS-0010.2].
     *
     * @param[in] precision
     *   Justification: Controls number of fractional digits emitted by `std::setprecision`.
     *   Usage: Passed to `std::setprecision(m_precision)` inside `rawToEng`.
     *   Consumed: Yes (copied scalar `int`).
     *   Prerequisites: Non-negative precision (`precision >= 0`).
     * @param[in] suffix
     *   Justification: Specifies engineering unit suffix (e.g., `"V"`, `"C"`, or `""`).
     *   Usage: Appended after a single space when non-empty and stripped in `engToRaw`.
     *   Consumed: Yes (moved-from via `std::move`).
     *   Prerequisites: Valid initialized `std::string` (may be empty).
     *
     * @pre precision >= 0
     * @post `m_precision == precision` and `m_suffix` stores the moved unit suffix.
     * @error
     *   - Validation / Contract (negative precision): Passed through to `std::setprecision` without eager exception.
     * @containment
     *   Approach: Simple member initialization using `std::move`.
     *   Limitations: Does not validate precision bounds eagerly at construction.
     */
    FormatCalibration(int precision, std::string suffix) : m_precision(precision), m_suffix(std::move(suffix)) {}

    /**
     * @brief Formats a numeric raw value as a fixed-precision engineering string.
     *
     * @details Converts `raw` via `toDouble(raw)`, formats with `std::fixed` and
     * `std::setprecision(m_precision)`, and appends `" " + m_suffix` if `!m_suffix.empty()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Validates numeric `raw` via `toDouble` before formatting output string.
     *
     * @param[in] raw
     *   Justification: Supplies raw numeric scalar to format for display or reporting.
     *   Usage: Converted via `toDouble(raw)` and streamed into `std::ostringstream`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Numeric `Variant` (`double`, `int64_t`, or `uint64_t`).
     *
     * @pre `raw` holds a numeric variant alternative.
     * @post Returned `Variant` holds formatted `std::string`.
     * @return Variant
     *   Justification: Delivers formatted engineering string representation.
     *   Usage: Caller extracts `std::string` via `std::get<std::string>`.
     *   Consumed: Value returned by move/copy.
     *   Prerequisites: Numeric `raw` input.
     *   Limitations: Uses C-locale decimal formatting of `std::ostringstream` (rounds at `m_precision`).
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(std::string)` formatted as `"<num>"` or `"<num> <m_suffix>"`.
     *   Error Behavior: Throws `std::invalid_argument` if `raw` is non-numeric.
     * @throws std::invalid_argument [Validation / Contract] Thrown by `toDouble(raw)` on non-numeric input.
     * @error
     *   - Validation / Contract (non-numeric `raw`): Throws `std::invalid_argument`; `this` unchanged.
     * @containment
     *   Approach: `toDouble` checks variant type before string stream formatting.
     *   Limitations: Round-trip `engToRaw(rawToEng(x))` loses digits beyond `m_precision`.
     */
    Variant rawToEng(const Variant& raw) const override {
        double v = toDouble(raw);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(m_precision) << v;
        // Append space-separated unit suffix when configured.
        if (!m_suffix.empty()) oss << " " << m_suffix;
        return oss.str();
    }

    /**
     * @brief Parses a formatted engineering string back into a raw `double` value.
     *
     * @details Validates `eng` holds `std::string`, strips a trailing `" " + m_suffix` when
     * present, and parses the numeric prefix via `std::stod`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0170
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Guards variant string alternative and substring bounds before suffix stripping and `std::stod`.
     *
     * @param[in] eng
     *   Justification: Supplies formatted engineering string (with optional unit suffix) to parse.
     *   Usage: Copied locally, stripped of trailing suffix if matched, and parsed with `std::stod`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: `Variant` holding a parseable numeric `std::string`.
     *
     * @pre std::holds_alternative<std::string>(eng)
     * @post Returned `Variant` holds parsed `double` value.
     * @return Variant
     *   Justification: Restores numeric raw value from formatted text representation.
     *   Usage: Caller unwraps `double` via `std::get<double>` or `toDouble`.
     *   Consumed: Value returned by copy.
     *   Prerequisites: `eng` holds a valid numeric `std::string`.
     *   Limitations: `std::stod` parses leading valid float prefix even if unknown trailing characters exist.
     *   Lifecycle: Independent caller-owned `Variant`.
     *   Modification / Deletion: Caller may mutate or discard returned `Variant`.
     *   Specific Values: `Variant(double)`.
     *   Error Behavior: Throws `std::invalid_argument` or `std::out_of_range` on non-string or unparseable input.
     * @throws std::invalid_argument [Validation / Contract] Thrown if `eng` is not `std::string` or `std::stod` cannot parse.
     * @throws std::out_of_range [Validation / Contract] Thrown if `std::stod` value overflows `double`.
     * @error
     *   - Validation / Contract (non-string `eng` or invalid float syntax): Throws `std::invalid_argument` / `std::out_of_range`.
     * @containment
     *   Approach: Checks `std::holds_alternative<std::string>` and `s.size() > m_suffix.size() + 1` before `substr`, preventing out-of-range substring throws.
     *   Limitations: Assumes single-space delimiter between value and suffix.
     */
    Variant engToRaw(const Variant& eng) const override {
        // Require string alternative before extracting formatted text.
        if (!std::holds_alternative<std::string>(eng)) throw std::invalid_argument("Expected string");
        std::string s = std::get<std::string>(eng);
        // Check string is long enough to hold value, separator, and configured unit suffix.
        if (!m_suffix.empty() && s.size() > m_suffix.size() + 1) {
            // Strip trailing unit suffix and separator space if present.
            if (s.substr(s.size() - m_suffix.size()) == m_suffix) {
                s = s.substr(0, s.size() - m_suffix.size() - 1);
            }
        }
        return std::stod(s);
    }
private:
    /**
     * @brief Fixed decimal digit precision passed to `std::setprecision`.
     * @details Immutable member initialized at construction.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Private member initialized in `FormatCalibration` constructor.
     */
    int m_precision;

    /**
     * @brief Optional engineering unit suffix appended after a single space in `rawToEng`.
     * @details Immutable member initialized via move in constructor.
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2
     * @compliance Private member initialized in `FormatCalibration` constructor.
     */
    std::string m_suffix;
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CALIBRATIONS_HPP
