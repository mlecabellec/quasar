/**
 * @file LuaBindings.cpp
 * @brief Implementation of Lua scripting bindings for Quasar calibration classes.
 * @details Defines bindCalibrationTypes() to expose polymorphic and named calibration types
 * as sol2 usertypes into caller-managed Lua virtual machines.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Enforces Doxygen file, namespace, and in-body documentation density standards (CS-0090.1, CS-0090.2, CS-0090.5, CS-0010.44).
 */
// Include calibration Lua binding declaration and calibration model headers.
#include "quasar/calibration/LuaBindings.hpp"
#include "quasar/calibration/NamedCalibration.hpp"
#include "quasar/calibration/Calibrations.hpp"

/**
 * @brief Root namespace for Quasar engineering-unit calibration types and bindings.
 * @details Contains the out-of-line implementation of bindCalibrationTypes.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Scopes binding implementation inside the calibration namespace (CS-0010.33).
 */
namespace quasar::calibration {

// Out-of-line definition of quasar::calibration::bindCalibrationTypes (formal CS-0090 contract in LuaBindings.hpp).
// Registers ICalibration, NamedLinearCalibration, NamedPolynomialCalibration, and CompositeCalibration in lua.
void bindCalibrationTypes(sol::state_view lua) {
    // Bind ICalibration to allow passing around generic variants
    // Wrap rawToEng and engToRaw via lambdas because calibration returns CalibrationValue variant.
    lua.new_usertype<ICalibration>("ICalibration",
        sol::no_constructor,
        "rawToEng", [](ICalibration& self, double raw) { return toDouble(self.rawToEng(raw)); },
        "engToRaw", [](ICalibration& self, double eng) { return toDouble(self.engToRaw(eng)); }
    );

    // Bind NamedLinearCalibration
    // Expose factory constructor, inheritance bases, and scale/offset read accessors to Lua.
    lua.new_usertype<NamedLinearCalibration>("NamedLinearCalibration",
        sol::constructors<NamedLinearCalibration(const std::string&, double, double)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        // Register scale and offset getter methods.
        "getScale", &NamedLinearCalibration::getScale,
        "getOffset", &NamedLinearCalibration::getOffset
    );

    // Bind NamedPolynomialCalibration
    // Expose coefficient vector constructor, inheritance bases, and coefficient getter to Lua.
    lua.new_usertype<NamedPolynomialCalibration>("NamedPolynomialCalibration",
        sol::constructors<NamedPolynomialCalibration(const std::string&, const std::vector<double>&)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        "getCoeffs", &NamedPolynomialCalibration::getCoeffs
    );

    // Bind CompositeCalibration
    // Expose composite container constructor, inheritance bases, and child-stage registration method.
    lua.new_usertype<CompositeCalibration>("CompositeCalibration",
        sol::constructors<CompositeCalibration(const std::string&)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        "addCalibration", &CompositeCalibration::addCalibration
    );
}

} // namespace quasar::calibration
