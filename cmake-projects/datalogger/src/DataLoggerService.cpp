#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/named/NamedMethod.hpp"

namespace quasar::datalogger {

DataLoggerService::DataLoggerService(const std::string& name, size_t ringBufferCapacity)
    : quasar::named::NamedService(name), 
      m_ringBuffer(std::make_shared<RingBuffer<LogEntry>>(ringBufferCapacity)) {
}

DataLoggerService::~DataLoggerService() = default;

std::shared_ptr<DataLoggerService> DataLoggerService::create(const std::string& name, size_t ringBufferCapacity, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct MakeSharedEnabler : public DataLoggerService {
        MakeSharedEnabler(const std::string& name, size_t capacity) : DataLoggerService(name, capacity) {}
    };
    
    auto service = std::make_shared<MakeSharedEnabler>(name, ringBufferCapacity);
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

std::shared_ptr<quasar::named::NamedObject> DataLoggerService::processRingBuffer(
    std::shared_ptr<quasar::named::NamedObject> owner, 
    std::shared_ptr<quasar::named::NamedObject> args) {
    
    (void)owner;
    (void)args;
    
    // Process all currently available items in the ring buffer
    while (auto optEntry = m_ringBuffer->pop()) {
        LogEntry entry = std::move(optEntry.value());
        
        std::unique_lock<std::timed_mutex> lock(m_pipelineMutex, std::defer_lock);
        if (lock.try_lock_for(std::chrono::milliseconds(10))) {
            bool drop = false;
            for (auto& filter : m_filters) {
                auto filtered = filter->process(std::move(entry));
                if (!filtered) {
                    drop = true;
                    break;
                }
                entry = std::move(filtered.value());
            }
            
            if (!drop) {
                for (auto& recorder : m_recorders) {
                    recorder->record(entry);
                }
            }
        }
    }
    
    return nullptr;
}

} // namespace quasar::datalogger
