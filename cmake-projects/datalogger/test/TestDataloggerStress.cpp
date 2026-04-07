#include <gtest/gtest.h>
#include "quasar/datalogger/RingBuffer.hpp"
#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/CsvFileWriter.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <cstdio>
#include <fstream>

using namespace quasar::datalogger;

TEST(TestDataloggerStress, RingBufferMultithread) {
    const size_t capacity = 10000;
    const int numProducers = 4;
    const int itemsPerProducer = 10000;
    RingBuffer<int> buffer(capacity);

    std::atomic<int> consumeCount{0};
    std::atomic<bool> producersDone{false};

    std::function<void()> consumer = [&]() {
        while (!producersDone || buffer.size() > 0) {
            if (std::optional<int> val = buffer.pop(std::chrono::milliseconds(1))) {
                consumeCount++;
            }
        }
    };

    std::thread consumerThread(consumer);

    std::vector<std::thread> producers;
    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < itemsPerProducer; ++j) {
                while (!buffer.push(j)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (std::thread& p : producers) {
        p.join();
    }
    producersDone = true;
    consumerThread.join();

    EXPECT_GT(consumeCount, 0);
}

TEST(TestDataloggerStress, ServiceHeavyLoad) {
    std::string testFilePath = "stress_log.csv";
    std::remove(testFilePath.c_str());

    std::shared_ptr<DataLoggerService> service = DataLoggerService::create("StressService", 50000);
    std::shared_ptr<CsvFileWriter> csvWriter = std::make_shared<CsvFileWriter>("StressCsv", testFilePath);
    service->addRecorder(csvWriter);

    service->setCycleTime(std::chrono::milliseconds(1));
    service->start();

    const int numThreads = 4;
    const int logsPerThread = 10000;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&service, i, logsPerThread]() {
            for (int j = 0; j < logsPerThread; ++j) {
                LogEntry e;
                e.timestamp = std::chrono::system_clock::now();
                e.payload = DataSample{"Stress/Thread" + std::to_string(i), static_cast<double>(j)};
                service->log(e);
            }
        });
    }

    for (std::thread& t : threads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    service->stop();
    csvWriter->flush();

    std::ifstream file(testFilePath);
    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }
    
    EXPECT_GT(lineCount, 0);
    std::remove(testFilePath.c_str());
}
