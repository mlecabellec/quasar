#ifndef QUASAR_CALIBRATION_ICALIBRATION_HPP
#define QUASAR_CALIBRATION_ICALIBRATION_HPP

#include <variant>
#include <string>
#include <cstdint>

namespace quasar::calibration {

/**
 * @typedef Variant
 * @brief Dynamic value type for data handling.
 */
using Variant = std::variant<std::monostate, bool, int64_t, uint64_t, double, std::string>;

/**
 * @interface ICalibration
 * @brief Abstract interface for bidirectional value transformation.
 * 
 * Enables high-fidelity conversion between raw system data and engineering units.
 */
class ICalibration {
public:
    virtual ~ICalibration() = default;

    /**
     * @brief Transforms a raw hardware value into an engineering value.
     * @param raw The raw input value.
     * @return The computed engineering value.
     */
    virtual Variant rawToEng(const Variant& raw) const = 0;

    /**
     * @brief Transforms an engineering value back into a raw hardware value.
     * @param eng The engineering input value.
     * @return The computed raw hardware value.
     */
    virtual Variant engToRaw(const Variant& eng) const = 0;
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_ICALIBRATION_HPP
