#include <gtest/gtest.h>
#include "quasar/datalogger/Log.hpp"
#include <fstream>
#include <string>
#include <cstdio>
#include <thread>
#include <filesystem>

using namespace quasar::datalogger;

class TestLogMacros : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset singleton to ensure a fresh start for every test case.
        DataLoggerService::resetInstance();
        
        // Use a dedicated test log file to avoid conflicts.
        testFilePath = "macro_test_log.csv";
        std::remove(testFilePath.c_str());
        
        // Explicitly initialize the logger for the test.
        DataLoggerService::initDefault(testFilePath, 1024);
    }

    void TearDown() override {
        // Stop and reset the logger.
        DataLoggerService::resetInstance();
        std::remove(testFilePath.c_str());
    }

    std::string testFilePath;
};

TEST_F(TestLogMacros, GlobalLoggingWorkflow) {
    // 1. Log multiple events via macros.
    LOG_INFO("Macro info message");
    LOG_WARN("Macro warning message");
    LOG_ERROR("Macro error message");
    
    // 2. Explicitly flush the singleton to ensure the background thread processes the logs.
    std::shared_ptr<DataLoggerService> inst = DataLoggerService::getInstance();
    if (inst) {
        inst->flush();
    }
    
    // Give OS a bit more time to sync the file to disk.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 3. Verify the file contents.
    std::ifstream file(testFilePath);
    ASSERT_TRUE(file.is_open());
    
    std::string line;
    int matchesFound = 0;
    
    // Skip header
    std::getline(file, line);
    
    // Check for our logged events in the CSV.
    while (std::getline(file, line)) {
        if (line.find("Macro info message") != std::string::npos) {
            matchesFound++;
        }
        if (line.find("Macro warning message") != std::string::npos) {
            matchesFound++;
        }
        if (line.find("Macro error message") != std::string::npos) {
            matchesFound++;
        }
    }
    
    EXPECT_EQ(matchesFound, 3);
}

TEST_F(TestLogMacros, SingletonInstanceStability) {
    // Both pointers should point to the same object.
    std::shared_ptr<DataLoggerService> inst1 = DataLoggerService::getInstance();
    std::shared_ptr<DataLoggerService> inst2 = DataLoggerService::getInstance();
    
    ASSERT_NE(inst1, nullptr);
    EXPECT_EQ(inst1, inst2);
}
