#pragma once

#include "quasar/named/NamedService.hpp"
#include "quasar/datalogger/RingBuffer.hpp"
#include "quasar/datalogger/LogEntry.hpp"
#include "quasar/datalogger/IRecorder.hpp"
#include "quasar/datalogger/IFilter.hpp"
#include <vector>
#include <memory>
#include <mutex>

namespace quasar::datalogger {

/**
 * @brief Orchestrates data logging, pulling from a ring buffer, 
 * applying filters, and dispatching to recorders.
 * 
 * @reference [TSK-20260303-001] Modular Data Logging and Acquisition System
 * @reference [FE-0160] Data Logging and Acquisition
 */
class DataLoggerService : public quasar::named::NamedService {
public:
    /**
     * @brief Factory method to create a DataLoggerService.
     * @param name The name of the service.
     * @param ringBufferCapacity Capacity of the internal ring buffer.
     * @param parent Optional parent in the hierarchy.
     */
    static std::shared_ptr<DataLoggerService> create(const std::string& name, size_t ringBufferCapacity, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Returns the global singleton instance of the DataLoggerService.
     * 
     * If the instance does not exist, it is created with default settings 
     * (name: "GlobalLogger", capacity: 2048, writer: "log.csv").
     * 
     * @return std::shared_ptr<DataLoggerService> The singleton instance.
     */
    static std::shared_ptr<DataLoggerService> getInstance();

    /**
     * @brief Explicitly initializes the global logger with custom settings.
     * 
     * Must be called before the first call to getInstance() or any LOG_* macro 
     * if custom settings are required.
     * 
     * @param filePath The path to the default CSV log file.
     * @param capacity The capacity of the ring buffer.
     */
    static void initDefault(const std::string& filePath = "log.csv", size_t capacity = 2048);

    /**
     * @brief Resets the singleton instance. Primarily for testing.
     * 
     * Stops the current service if it is running.
     */
    static void resetInstance();

    /**
     * @brief Destructor.
     */
    ~DataLoggerService() override;

    /**
     * @brief Adds a recorder to the service pipeline.
     * @param recorder The recorder to attach.
     */
    void addRecorder(std::shared_ptr<IRecorder> recorder);

    /**
     * @brief Adds a filter to the service pipeline.
     * @param filter The filter to attach.
     */
    void addFilter(std::shared_ptr<IFilter> filter);
    
    /**
     * @brief Pushes a log entry asynchronously into the ring buffer.
     * @param entry The log entry to queue.
     */
    void log(const LogEntry& entry);

    /**
     * @brief Convenience method to log an arbitrary event.
     * @param level The log severity level.
     * @param message The log message.
     */
    void logEvent(LogLevel level, const std::string& message);

    /**
     * @brief Flushes all attached recorders.
     */
    void flush();

    /**
     * @brief Returns the class type.
     */
    std::string getType() const override;

public:
    /**
     * @brief Protected constructor made public for std::make_shared.
     */
    DataLoggerService(const std::string& name, size_t ringBufferCapacity);

private:
    std::shared_ptr<quasar::named::NamedObject> processRingBuffer(
        std::shared_ptr<quasar::named::NamedObject> owner, 
        std::shared_ptr<quasar::named::NamedObject> args);

    std::shared_ptr<RingBuffer<LogEntry>> m_ringBuffer;
    
    std::timed_mutex m_pipelineMutex;
    std::vector<std::shared_ptr<IRecorder>> m_recorders;
    std::vector<std::shared_ptr<IFilter>> m_filters;
};

} // namespace quasar::datalogger
