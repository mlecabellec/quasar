#include <gtest/gtest.h>
#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/MathFilter.hpp"
#include <chrono>
#include <thread>

using namespace quasar::datalogger;

class DummyRecorder : public IRecorder {
public:
    std::vector<LogEntry> entries;
    void record(const LogEntry& entry) override {
        entries.push_back(entry);
    }
    void flush() override {}
};

TEST(TestDataLoggerService, PipelineTest) {
    auto service = DataLoggerService::create("LogService", 1024);
    auto recorder = std::make_shared<DummyRecorder>();
    service->addRecorder(recorder);
    
    auto filter = std::make_shared<MathFilter>("Sensor", 2.0, 10.0);
    service->addFilter(filter);
    
    service->setCycleTime(std::chrono::milliseconds(10));
    service->start();
    
    LogEntry e;
    e.timestamp = std::chrono::system_clock::now();
    e.payload = DataSample{"Sensor", 5.0};
    service->log(e);
    
    // Arbitrary event logging
    service->logEvent(LogLevel::INFO, "Processing started");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    service->stop();
    
    ASSERT_EQ(recorder->entries.size(), 2);
    
    auto& dataEntry = recorder->entries[0];
    ASSERT_TRUE(std::holds_alternative<DataSample>(dataEntry.payload));
    auto& ds = std::get<DataSample>(dataEntry.payload);
    ASSERT_EQ(ds.sourcePath, "Sensor");
    ASSERT_TRUE(std::holds_alternative<double>(ds.value));
    EXPECT_DOUBLE_EQ(std::get<double>(ds.value), 20.0); // 5 * 2 + 10

    auto& evEntry = recorder->entries[1];
    ASSERT_TRUE(std::holds_alternative<EventLog>(evEntry.payload));
    auto& ev = std::get<EventLog>(evEntry.payload);
    EXPECT_EQ(ev.level, LogLevel::INFO);
    EXPECT_EQ(ev.message, "Processing started");
}
