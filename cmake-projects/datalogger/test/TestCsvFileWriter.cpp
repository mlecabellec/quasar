#include <gtest/gtest.h>
#include "quasar/datalogger/CsvFileWriter.hpp"
#include "quasar/datalogger/LogEntry.hpp"
#include <fstream>
#include <string>
#include <cstdio>

using namespace quasar::datalogger;

class TestCsvFileWriter : public ::testing::Test {
protected:
    void SetUp() override {
        testFilePath = "test_log.csv";
        std::remove(testFilePath.c_str());
    }

    void TearDown() override {
        std::remove(testFilePath.c_str());
    }

    std::string testFilePath;
};

TEST_F(TestCsvFileWriter, WriteEventAndData) {
    {
        CsvFileWriter writer("test_writer", testFilePath);
        
        LogEntry e1;
        e1.timestamp = std::chrono::system_clock::now();
        EventLog ev{LogLevel::INFO, "System started"};
        e1.payload = ev;
        
        LogEntry e2;
        e2.timestamp = std::chrono::system_clock::now();
        DataSample ds{"Sensor/Temp", 24.5};
        e2.payload = ds;
        
        writer.record(e1);
        writer.record(e2);
        
        writer.flush();
    } // Writer destroyed, file closed

    std::ifstream file(testFilePath);
    ASSERT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Timestamp,Type,Source/Level,Value/Message");
    
    std::getline(file, line);
    EXPECT_NE(line.find("EVENT"), std::string::npos);
    EXPECT_NE(line.find("System started"), std::string::npos);
    
    std::getline(file, line);
    EXPECT_NE(line.find("DATA"), std::string::npos);
    EXPECT_NE(line.find("Sensor/Temp"), std::string::npos);
    EXPECT_NE(line.find("24.5"), std::string::npos);
}
