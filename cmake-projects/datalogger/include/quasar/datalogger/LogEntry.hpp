#pragma once

#include <chrono>
#include <string>
#include <variant>
#include <cstdint>

namespace quasar::datalogger {

/**
 * @brief Severity levels for event logging.
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Represents an arbitrary log event with a message and severity level.
 */
struct EventLog {
    /** @brief The severity level of the event. */
    LogLevel level;
    /** @brief The formatted message for the event. */
    std::string message;
};

/**
 * @brief Represents a sampled data point from the system.
 */
struct DataSample {
    /** @brief The source path or identifier of the data. */
    std::string sourcePath;
    /** @brief The value of the sample, accommodating common types. */
    std::variant<double, int64_t, std::string, bool> value;
};

/**
 * @brief A unified record entry containing either an event or a data sample.
 */
struct LogEntry {
    /** @brief The timestamp when the entry was created or recorded. */
    std::chrono::system_clock::time_point timestamp;
    /** @brief The payload containing either an EventLog or a DataSample. */
    std::variant<EventLog, DataSample> payload;
};

} // namespace quasar::datalogger
