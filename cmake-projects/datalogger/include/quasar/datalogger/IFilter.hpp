#pragma once

#include "quasar/datalogger/LogEntry.hpp"
#include <optional>

namespace quasar::datalogger {

/**
 * @brief Abstract interface for the pluggable data pipeline filters.
 */
class IFilter {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IFilter() = default;

    /**
     * @brief Processes a log entry, potentially modifying it or dropping it.
     * @param entry The input log entry.
     * @return The modified log entry, or nullopt if the entry is dropped.
     */
    [[nodiscard]] virtual std::optional<LogEntry> process(LogEntry entry) = 0;
};

} // namespace quasar::datalogger
