#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "quasar/coretypes/IntegerTypes.hpp"
#include "quasar/coretypes/FloatingPointTypes.hpp"

using namespace quasar::coretypes;

TEST(CoreTypesThreadSafety, IntegerImmutable) {
    Int sharedInt(42);
    std::atomic<bool> stop{false};
    
    auto reader = [&]() {
        while (!stop) {
            EXPECT_EQ(sharedInt.toInt(), 42);
            Int copy = sharedInt;
            EXPECT_EQ(copy.toInt(), 42);
        }
    };
    
    std::vector<std::thread> threads;
    for (int i=0; i<10; ++i) {
        threads.emplace_back(reader);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;
    for (auto& t : threads) t.join();
}

TEST(CoreTypesThreadSafety, FloatingPointImmutable) {
    Double sharedDouble(3.14);
    std::atomic<bool> stop{false};
    
    auto reader = [&]() {
        while (!stop) {
            EXPECT_DOUBLE_EQ(sharedDouble.toDouble(), 3.14);
            Double copy = sharedDouble;
            EXPECT_DOUBLE_EQ(copy.toDouble(), 3.14);
        }
    };
    
    std::vector<std::thread> threads;
    for (int i=0; i<10; ++i) {
        threads.emplace_back(reader);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;
    for (auto& t : threads) t.join();
}
