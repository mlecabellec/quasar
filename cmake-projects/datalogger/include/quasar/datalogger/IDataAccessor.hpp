#pragma once

#include "quasar/datalogger/LogEntry.hpp"
#include <optional>

namespace quasar::datalogger {

/**
 * @brief Interface for accessing and sampling data from the system.
 */
class IDataAccessor {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IDataAccessor() = default;

    /**
     * @brief Samples the current value of the target object.
     * @return An optional DataSample. Nullopt if sampling fails.
     */
    [[nodiscard]] virtual std::optional<DataSample> sample() const = 0;
};

} // namespace quasar::datalogger
