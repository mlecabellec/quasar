#include <gtest/gtest.h>
#include "quasar/datalogger/RingBuffer.hpp"
#include "quasar/datalogger/LogEntry.hpp"
#include <thread>
#include <vector>

using namespace quasar::datalogger;

TEST(TestDataloggerRingBuffer, BasicPushPop) {
    RingBuffer<int> buffer(5);
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_EQ(buffer.size(), 2);

    std::optional<int> val1 = buffer.pop();
    EXPECT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), 1);

    std::optional<int> val2 = buffer.pop();
    EXPECT_TRUE(val2.has_value());
    EXPECT_EQ(val2.value(), 2);

    EXPECT_EQ(buffer.size(), 0);
}

TEST(TestDataloggerRingBuffer, CapacityOverwrite) {
    RingBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4); // Overwrites 1

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.pop().value(), 2);
    EXPECT_EQ(buffer.pop().value(), 3);
    EXPECT_EQ(buffer.pop().value(), 4);
    EXPECT_FALSE(buffer.pop().has_value());
}
