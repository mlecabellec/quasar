#pragma once

#include "quasar/datalogger/ADevRecorder.hpp"
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace quasar::datalogger {

/**
 * @brief High-performance CSV file writer for logging data and events.
 * 
 * Uses double-buffering and a background thread to prevent blocking 
 * the data acquisition pipeline during file I/O operations.
 */
class CsvFileWriter : public ADevRecorder {
public:
    /**
     * @brief Constructs a new CsvFileWriter.
     * @param name The name of the recorder object.
     * @param filePath The path to the CSV file.
     * @param parent Optional parent NamedObject.
     */
    CsvFileWriter(const std::string& name, const std::string& filePath, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor. Ensures all background writes complete.
     */
    ~CsvFileWriter() override;

    /**
     * @brief Pushes a log entry into the active buffer.
     * @param entry The log entry to record.
     */
    void record(const LogEntry& entry) override;

    /**
     * @brief Flushes any pending data to disk.
     */
    void flush() override;

private:
    void writerLoop();
    void writeBuffer(const std::vector<LogEntry>& buffer);
    std::string formatIso8601(const std::chrono::system_clock::time_point& tp) const;

    std::string m_filePath;
    std::ofstream m_file;

    size_t m_chunkSize;
    std::vector<LogEntry> m_frontBuffer;
    std::vector<LogEntry> m_backBuffer;

    std::timed_mutex m_mutex;
    std::condition_variable_any m_cv;
    std::thread m_writerThread;
    std::atomic<bool> m_running;
    bool m_swapReady;
};

} // namespace quasar::datalogger
