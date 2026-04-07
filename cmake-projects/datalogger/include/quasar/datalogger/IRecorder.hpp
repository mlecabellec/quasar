#pragma once

#include "quasar/datalogger/LogEntry.hpp"

namespace quasar::datalogger {

/**
 * @brief Abstract interface for logging backends and recorders.
 */
class IRecorder {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IRecorder() = default;

    /**
     * @brief Writes a single log entry to the backend.
     * @param entry The log entry to write.
     */
    virtual void record(const LogEntry& entry) = 0;

    /**
     * @brief Flushes any buffered data to the underlying storage.
     */
    virtual void flush() = 0;
};

} // namespace quasar::datalogger
