#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/CsvFileWriter.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/coretypes/Constants.hpp"
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
    service->setSelf(service);
    if (parent) {
        service->setParent(parent);
    }
    
    std::weak_ptr<DataLoggerService> weakSvc = service;
    quasar::named::NamedMethod::create("run",
        [weakSvc](std::shared_ptr<quasar::named::NamedObject> owner, std::shared_ptr<quasar::named::NamedObject> args) {
            if (std::shared_ptr<DataLoggerService> svc = weakSvc.lock()) {
                return svc->processRingBuffer(owner, args);
            }
            return std::shared_ptr<quasar::named::NamedObject>(nullptr);
        }, service);
        
    return service;
}

std::string DataLoggerService::getType() const {
    return "DataLoggerService";
}

void DataLoggerService::addRecorder(std::shared_ptr<IRecorder> recorder) {
    std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
    if (lock.try_lock_for(quasar::coretypes::DEFAULT_MUTEX_TIMEOUT)) {
        m_recorders.push_back(std::move(recorder));
    }
}

void DataLoggerService::addFilter(std::shared_ptr<IFilter> filter) {
    std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
    if (lock.try_lock_for(quasar::coretypes::DEFAULT_MUTEX_TIMEOUT)) {
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
        const size_t limit = 10000;
        size_t count = 0;
        for (std::vector<std::shared_ptr<IRecorder>>::iterator it = m_recorders.begin(); it != m_recorders.end(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in DataLoggerService::flush");
            std::shared_ptr<IRecorder>& recorder = *it;
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
    const size_t limitPop = 1000000;
    size_t countPop = 0;
    while (std::optional<LogEntry> optEntry = m_ringBuffer->pop()) {
        if (++countPop > limitPop) throw std::runtime_error("Pop loop limit exceeded in processRingBuffer");
        LogEntry entry = std::move(optEntry.value());
        
        std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
        if (lock.try_lock_for(quasar::coretypes::DEFAULT_MUTEX_TIMEOUT)) {
            bool drop = false;
            const size_t limitFilters = 1000;
            size_t countFilters = 0;
            for (std::vector<std::shared_ptr<IFilter>>::iterator it = m_filters.begin(); it != m_filters.end(); ++it) {
                if (++countFilters > limitFilters) throw std::runtime_error("Filter loop limit exceeded in processRingBuffer");
                std::shared_ptr<IFilter>& filter = *it;
                std::optional<LogEntry> filtered = filter->process(std::move(entry));
                if (!filtered) {
                    drop = true;
                    break;
                }
                entry = std::move(filtered.value());
            }
            
            if (!drop) {
                const size_t limitRecorders = 1000;
                size_t countRecorders = 0;
                for (std::vector<std::shared_ptr<IRecorder>>::iterator it = m_recorders.begin(); it != m_recorders.end(); ++it) {
                    if (++countRecorders > limitRecorders) throw std::runtime_error("Recorder loop limit exceeded in processRingBuffer");
                    std::shared_ptr<IRecorder>& recorder = *it;
                    recorder->record(entry);
                }
            }
        }
    }
    
    return nullptr;
}

} // namespace quasar::datalogger
