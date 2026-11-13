/**
 * @file CalibrationManager.hpp
 * @brief Declares CalibrationManager container for managing and looking up NamedCalibration nodes.
 *
 * @details Specializes `quasar::named::NamedObject` as a named registry container providing
 * bounded lookup (`getCalibration`) over child `NamedCalibration` objects.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Enforces complete Doxygen coverage, bounded loop iteration, and control comments.
 */

#ifndef QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
#define QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP

// Quasar NamedObject base and NamedCalibration interface headers.
#include "quasar/named/NamedObject.hpp"
#include "NamedCalibration.hpp"

/**
 * @brief Quasar engineering-unit calibration namespace.
 *
 * @details Contains `CalibrationManager` for organizing named calibrations in a hierarchy.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Houses `CalibrationManager` within `quasar::calibration`.
 */
namespace quasar::calibration {

/**
 * @class CalibrationManager
 * @brief Manages a collection of calibrations.
 *
 * @details Inherits from `quasar::named::NamedObject` and provides typed child resolution via
 * `getCalibration(calibName)` with a hard safety bound of `1000000` children.
 *
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.9
 * @exposed
 * @invariant Child `NamedCalibration` nodes are stored in `NamedObject` child hierarchy.
 * @compliance Uses bounded child traversal and explicit dynamic type checking.
 */
class CalibrationManager : public quasar::named::NamedObject {
public:
    /**
     * @brief Constructs a CalibrationManager.
     *
     * @details Initializes base `NamedObject` with the manager's identifier `name`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0010.31, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance `explicit` single-argument constructor.
     *
     * @param[in] name
     *   Justification: Assigns the unique `NamedObject` name for this manager container.
     *   Usage: Forwarded to `NamedObject(name)`.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Non-empty node name.
     *
     * @pre !name.empty()
     * @post Manager constructed with `getName() == name` and empty child list.
     * @throws std::exception [Resource / Allocation] Propagates if `NamedObject` construction throws.
     * @error
     *   - Resource / Allocation (name allocation OOM): Propagates cleanly.
     * @containment
     *   Approach: Delegates directly to `NamedObject` constructor.
     *   Limitations: None.
     */
    explicit CalibrationManager(const std::string& name) : NamedObject(name) {}

    /**
     * @brief Returns the type of the object.
     *
     * @details Overrides `NamedObject::getType()` with `"CalibrationManager"`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Deterministic reflexive type tag.
     *
     * @post Returned string equals `"CalibrationManager"`.
     * @return std::string
     *   Justification: Identifies manager node type for Quasar reflection.
     *   Usage: Caller compares returned string against `"CalibrationManager"`.
     *   Consumed: Value returned by move/NRVO.
     *   Prerequisites: None.
     *   Limitations: Constant literal return.
     *   Lifecycle: Independent caller-owned `std::string`.
     *   Modification / Deletion: Caller may mutate or discard freely.
     *   Specific Values: `"CalibrationManager"`.
     *   Error Behavior: Propagates `std::bad_alloc` only under extreme OOM.
     * @throws std::bad_alloc [Resource / Allocation] Possible on string allocation failure.
     * @error
     *   - Resource / Allocation (OOM): Propagates `std::bad_alloc`.
     * @containment
     *   Approach: Constant literal return.
     *   Limitations: Non-`noexcept` due to base virtual signature.
     */
    std::string getType() const override { return "CalibrationManager"; }

    /**
     * @brief Retrieves a calibration by name.
     *
     * @details Iterates through `getChildren()` up to hard safety limit `1000000`, returning the
     * first child whose name matches `calibName` and whose type satisfies `is<NamedCalibration>()`.
     *
     * @task TSK-20261113-003, TSK-20260311-010
     * @feature FE-0050, FE-0160, FE-0210
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
     * @exposed
     * @compliance Enforces bounded traversal (`limit = 1000000`) and checked `is<NamedCalibration>()` cast.
     *
     * @param[in] calibName
     *   Justification: Identifies the target `NamedCalibration` child to retrieve.
     *   Usage: Compared against `child->getName()` during child iteration.
     *   Consumed: No (read-only const reference).
     *   Prerequisites: Valid `std::string` name (empty or missing name yields `nullptr`).
     *
     * @pre `getChildren().size() <= 1000000`
     * @post Returned `shared_ptr` is non-null and named `calibName` if present, or `nullptr` if absent.
     * @return std::shared_ptr<NamedCalibration>
     *   Justification: Grants shared access to registered calibration object by name.
     *   Usage: Caller checks `ret != nullptr` before invoking `rawToEng`/`engToRaw`.
     *   Consumed: Shared ownership via `std::shared_ptr` (does not remove child from manager).
     *   Prerequisites: Caller must check for `nullptr` return before dereferencing.
     *   Limitations: Bounded to 1,000,000 direct children; performs linear O(N) scan.
     *   Lifecycle: Valid while returned `shared_ptr` or manager parent reference exists.
     *   Modification / Deletion: Caller may invoke calibration methods or release `shared_ptr`.
     *   Specific Values: Matching `std::shared_ptr<NamedCalibration>`, or `nullptr` when not found.
     *   Error Behavior: Returns `nullptr` if absent; throws `std::runtime_error` if child count > 1,000,000.
     * @throws std::runtime_error [Resource / Fatal] If loop limit is exceeded (> 1,000,000 children).
     * @error
     *   - Resource / Fatal (child count > 1,000,000): Throws `std::runtime_error`; manager untouched.
     * @containment
     *   Approach: Explicit safety counter `limit = 1000000` and `child->is<NamedCalibration>()` type guard.
     *   Limitations: Does not recurse into nested sub-containers.
     */
    std::shared_ptr<NamedCalibration> getCalibration(const std::string& calibName) const {
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000; // Hard limit for safety [CS-0010.37]
        size_t count = 0;
        // Search direct children bounded by hard limit 1000000 [CS-0010.37].
        for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
            // Throw immediately if child count exceeds hard iteration limit.
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in getCalibration");
            std::shared_ptr<NamedObject> child = *it;
            // Return typed handle when both child name and NamedCalibration type match.
            if (child->getName() == calibName && child->is<NamedCalibration>()) {
                return child->as<NamedCalibration>();
            }
        }
        return nullptr;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
