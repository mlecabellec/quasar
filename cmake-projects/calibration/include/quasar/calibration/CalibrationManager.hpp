#ifndef QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
#define QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP

#include "quasar/named/NamedObject.hpp"
#include "NamedCalibration.hpp"

namespace quasar::calibration {

class CalibrationManager : public quasar::named::NamedObject {
public:
    explicit CalibrationManager(const std::string& name) : NamedObject(name) {}

    std::string getType() const override { return "CalibrationManager"; }

    std::shared_ptr<NamedCalibration> getCalibration(const std::string& calibName) const {
        for (auto child : getChildren()) {
            if (child->getName() == calibName && child->is<NamedCalibration>()) {
                return child->as<NamedCalibration>();
            }
        }
        return nullptr;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
