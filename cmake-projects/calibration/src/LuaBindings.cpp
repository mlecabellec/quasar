#include "quasar/calibration/LuaBindings.hpp"
#include "quasar/calibration/NamedCalibration.hpp"
#include "quasar/calibration/Calibrations.hpp"

namespace quasar::calibration {

void bindCalibrationTypes(sol::state_view lua) {
    // Bind ICalibration to allow passing around generic variants
    lua.new_usertype<ICalibration>("ICalibration",
        sol::no_constructor,
        "rawToEng", [](ICalibration& self, double raw) { return toDouble(self.rawToEng(raw)); },
        "engToRaw", [](ICalibration& self, double eng) { return toDouble(self.engToRaw(eng)); }
    );

    // Bind NamedLinearCalibration
    lua.new_usertype<NamedLinearCalibration>("NamedLinearCalibration",
        sol::constructors<NamedLinearCalibration(const std::string&, double, double)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        "getScale", &NamedLinearCalibration::getScale,
        "getOffset", &NamedLinearCalibration::getOffset
    );

    // Bind NamedPolynomialCalibration
    lua.new_usertype<NamedPolynomialCalibration>("NamedPolynomialCalibration",
        sol::constructors<NamedPolynomialCalibration(const std::string&, const std::vector<double>&)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        "getCoeffs", &NamedPolynomialCalibration::getCoeffs
    );

    // Bind CompositeCalibration
    lua.new_usertype<CompositeCalibration>("CompositeCalibration",
        sol::constructors<CompositeCalibration(const std::string&)>(),
        sol::base_classes, sol::bases<ICalibration, quasar::named::NamedObject>(),
        "addCalibration", &CompositeCalibration::addCalibration
    );
}

} // namespace quasar::calibration
