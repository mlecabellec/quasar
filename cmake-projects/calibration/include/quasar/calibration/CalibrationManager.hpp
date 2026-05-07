#ifndef QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
#define QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP

#include "quasar/named/NamedObject.hpp"
#include "NamedCalibration.hpp"

namespace quasar::calibration {

/**
 * @class CalibrationManager
 * @brief Manages a collection of calibrations.
 */
class CalibrationManager : public quasar::named::NamedObject {
public:
    /**
     * @brief Constructs a CalibrationManager.
     * @param name The name of the manager.
     */
    explicit CalibrationManager(const std::string& name) : NamedObject(name) {}

    /**
     * @brief Returns the type of the object.
     * @return "CalibrationManager"
     */
    std::string getType() const override { return "CalibrationManager"; }

    /**
     * @brief Retrieves a calibration by name.
     * @param calibName The name of the calibration to find.
     * @return Shared pointer to the calibration, or nullptr if not found.
     * @throws std::runtime_error If loop limit is exceeded.
     */
    std::shared_ptr<NamedCalibration> getCalibration(const std::string& calibName) const {
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000; // Hard limit for safety [CS-0010.37]
        size_t count = 0;
        for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in getCalibration");
            std::shared_ptr<NamedObject> child = *it;
            if (child->getName() == calibName && child->is<NamedCalibration>()) {
                return child->as<NamedCalibration>();
            }
        }
        return nullptr;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CALIBRATIONMANAGER_HPP
