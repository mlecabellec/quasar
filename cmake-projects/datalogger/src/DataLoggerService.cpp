#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/CsvFileWriter.hpp"
#include "quasar/named/NamedMethod.hpp"
#include <mutex>

namespace quasar::datalogger {

// Static pointer for the global singleton instance.
static std::shared_ptr<DataLoggerService> g_instance = nullptr;

// Mutex for safe singleton initialization. Recursive to allow initDefault to be called from getInstance.
static std::recursive_timed_mutex g_initMutex;

DataLoggerService::DataLoggerService(const std::string& name, size_t ringBufferCapacity)
    : quasar::named::NamedService(name), 
      m_ringBuffer(std::make_shared<RingBuffer<LogEntry>>(ringBufferCapacity)) {
}

DataLoggerService::~DataLoggerService() = default;

std::shared_ptr<DataLoggerService> DataLoggerService::getInstance() {
    // Acquire the initialization mutex with a timeout.
    std::unique_lock<std::recursive_timed_mutex> lock(g_initMutex, std::defer_lock);
    
    // Check if the mutex was acquired successfully.
    if (!lock.try_lock_for(std::chrono::seconds(1))) {
        // If we cannot acquire the lock, return the current state (might be null).
        return g_instance;
    }

    // Initialize with default settings if not already done.
    if (!g_instance) {
        initDefault();
    }
    
    // Return the singleton instance.
    return g_instance;
}

void DataLoggerService::initDefault(const std::string& filePath, size_t capacity) {
    // Ensure exclusive access during initialization.
    std::unique_lock<std::recursive_timed_mutex> lock(g_initMutex, std::defer_lock);
    
    // Try to acquire lock for 1 second.
    if (lock.try_lock_for(std::chrono::seconds(1))) {
        // Only initialize if not already set.
        if (!g_instance) {
            // Create the service instance with the GlobalLogger name.
            g_instance = create("GlobalLogger", capacity);
            
            // Add a default CSV recorder to ensure data is persisted.
            std::shared_ptr<CsvFileWriter> recorder = std::make_shared<CsvFileWriter>("DefaultCsvWriter", filePath);
            g_instance->addRecorder(std::move(recorder));
            
            // Start the service to begin processing logs immediately.
            g_instance->start();
        }
    }
}

void DataLoggerService::resetInstance() {
    // Acquire the initialization mutex.
    std::unique_lock<std::recursive_timed_mutex> lock(g_initMutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::seconds(1))) {
        if (g_instance) {
            // Stop background threads.
            g_instance->stop();
            // Clear the shared pointer to allow destruction.
            g_instance = nullptr;
        }
    }
}

std::shared_ptr<DataLoggerService> DataLoggerService::create(const std::string& name, size_t ringBufferCapacity, std::shared_ptr<quasar::named::NamedObject> parent) {
    std::shared_ptr<DataLoggerService> service = std::make_shared<DataLoggerService>(name, ringBufferCapacity);
    if (parent) {
        service->setParent(parent);
    }
    
    quasar::named::NamedMethod::create("run",
        [svc = service.get()](std::shared_ptr<quasar::named::NamedObject> owner, std::shared_ptr<quasar::named::NamedObject> args) {
            return svc->processRingBuffer(owner, args);
        }, service);
        
    return service;
}

std::string DataLoggerService::getType() const {
    return "DataLoggerService";
}

void DataLoggerService::addRecorder(std::shared_ptr<IRecorder> recorder) {
    std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::milliseconds(100))) {
        m_recorders.push_back(std::move(recorder));
    }
}

void DataLoggerService::addFilter(std::shared_ptr<IFilter> filter) {
    std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::milliseconds(100))) {
        m_filters.push_back(std::move(filter));
    }
}

void DataLoggerService::log(const LogEntry& entry) {
    m_ringBuffer->push(entry);
}

void DataLoggerService::logEvent(LogLevel level, const std::string& message) {
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.payload = EventLog{level, message};
    log(entry);
}

void DataLoggerService::flush() {
    // Manually run the processRingBuffer once to move items from the ring buffer to recorders.
    processRingBuffer(nullptr, nullptr);

    // Acquire the pipeline mutex with a timeout.
    std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
    if (lock.try_lock_for(std::chrono::seconds(1))) {
        // Iterate through all recorders and signal them to flush.
        for (std::shared_ptr<IRecorder>& recorder : m_recorders) {
            recorder->flush();
        }
    }
}

std::shared_ptr<quasar::named::NamedObject> DataLoggerService::processRingBuffer(
    std::shared_ptr<quasar::named::NamedObject> owner, 
    std::shared_ptr<quasar::named::NamedObject> args) {
    
    (void)owner;
    (void)args;
    
    // Process all currently available items in the ring buffer
    while (std::optional<LogEntry> optEntry = m_ringBuffer->pop()) {
        LogEntry entry = std::move(optEntry.value());
        
        std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
        if (lock.try_lock_for(std::chrono::milliseconds(100))) {
            bool drop = false;
            for (std::shared_ptr<IFilter>& filter : m_filters) {
                std::optional<LogEntry> filtered = filter->process(std::move(entry));
                if (!filtered) {
                    drop = true;
                    break;
                }
                entry = std::move(filtered.value());
            }
            
            if (!drop) {
                for (std::shared_ptr<IRecorder>& recorder : m_recorders) {
                    recorder->record(entry);
                }
            }
        }
    }
    
    return nullptr;
}

} // namespace quasar::datalogger
