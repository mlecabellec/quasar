#include "quasar/datalogger/CsvFileWriter.hpp"
#include <iomanip>
#include <sstream>
#include <type_traits>
#include <iostream>

namespace quasar::datalogger {

CsvFileWriter::CsvFileWriter(const std::string& name, const std::string& filePath)
    : ADevRecorder(name), m_filePath(filePath), m_chunkSize(1000), m_running(true), m_swapReady(false) {
    
    m_file.open(m_filePath, std::ios::out | std::ios::app);
    if (!m_file.is_open()) {
        throw std::runtime_error("CsvFileWriter failed to open file: " + m_filePath);
    }
    
    // Write header
    m_file << "Timestamp,Type,Source/Level,Value/Message\n";
    
    m_frontBuffer.reserve(m_chunkSize);
    m_backBuffer.reserve(m_chunkSize);
    
    m_writerThread = std::thread(&CsvFileWriter::writerLoop, this);
}

CsvFileWriter::~CsvFileWriter() {
    flush();
    m_running = false;
    m_cv.notify_all();
    if (m_writerThread.joinable()) {
        m_writerThread.join();
    }
    if (m_file.is_open()) {
        m_file.close();
    }
}

void CsvFileWriter::record(const LogEntry& entry) {
    std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::milliseconds(50))) {
        m_frontBuffer.push_back(entry);
        if (m_frontBuffer.size() >= m_chunkSize) {
            m_swapReady = true;
            m_cv.notify_one();
        }
    }
}

void CsvFileWriter::flush() {
    std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::seconds(1))) {
        m_swapReady = true;
        m_cv.notify_one();
    }
    // Briefly wait for back buffer to empty
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void CsvFileWriter::writerLoop() {
    while (m_running) {
        std::unique_lock<std::timed_mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_swapReady || !m_running; });
        
        if (m_swapReady) {
            std::swap(m_frontBuffer, m_backBuffer);
            m_swapReady = false;
            lock.unlock(); // Unlock while writing
            
            if (!m_backBuffer.empty()) {
                writeBuffer(m_backBuffer);
                m_backBuffer.clear();
            }
        }
    }
}

std::string CsvFileWriter::formatIso8601(const std::chrono::system_clock::time_point& tp) const {
    std::time_t time_c = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    std::tm tm_buf;
    localtime_r(&time_c, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

void CsvFileWriter::writeBuffer(const std::vector<LogEntry>& buffer) {
    for (const LogEntry& entry : buffer) {
        m_file << formatIso8601(entry.timestamp) << ",";
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, EventLog>) {
                m_file << "EVENT," << static_cast<int>(arg.level) << ",\"" << arg.message << "\"";
            } else if constexpr (std::is_same_v<T, DataSample>) {
                m_file << "DATA," << arg.sourcePath << ",";
                std::visit([this](auto&& val) {
                    m_file << val;
                }, arg.value);
            }
        }, entry.payload);
        m_file << "\n";
    }
    m_file.flush();
}

} // namespace quasar::datalogger
