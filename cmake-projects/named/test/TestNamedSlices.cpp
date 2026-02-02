#include <gtest/gtest.h>
#include "quasar/named/NamedBufferSlice.hpp"
#include "quasar/named/NamedBitBufferSlice.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BitBuffer.hpp"

using namespace quasar::named;
using namespace quasar::coretypes;

TEST(NamedBufferSliceTest, CreationAndUsage) {
    auto buf = std::make_shared<Buffer>(10);
    buf->set(0, 0xAA);
    buf->set(1, 0xBB);
    
    auto slice = NamedBufferSlice::create("slice1", buf, 0, 5);
    EXPECT_EQ(slice->getName(), "slice1");
    EXPECT_EQ(slice->size(), 5);
    EXPECT_EQ(slice->get(0), 0xAA);
    
    auto subSlice = slice->sliceView(1, 2);
    EXPECT_EQ(subSlice->getName(), "slice1_slice");
    EXPECT_EQ(subSlice->size(), 2);
    EXPECT_EQ(subSlice->get(0), 0xBB);
}

TEST(NamedBitBufferSliceTest, CreationAndUsage) {
    auto bb = std::make_shared<BitBuffer>(16);
    bb->setBit(0, true);
    bb->setBit(2, true);
    
    auto slice = NamedBitBufferSlice::create("bitslice1", bb, 0, 8);
    EXPECT_EQ(slice->getName(), "bitslice1");
    EXPECT_EQ(slice->size(), 8);
    EXPECT_TRUE(slice->getBit(0));
    EXPECT_FALSE(slice->getBit(1));
    EXPECT_TRUE(slice->getBit(2));
    
    auto subSlice = slice->sliceView(1, 4);
    EXPECT_EQ(subSlice->getName(), "bitslice1_slice");
    EXPECT_EQ(subSlice->size(), 4);
    EXPECT_FALSE(subSlice->getBit(0)); // bit 1 of original
    EXPECT_TRUE(subSlice->getBit(1));  // bit 2 of original
}

TEST(NamedBufferSliceTest, Clone) {
    auto buf = std::make_shared<Buffer>(10);
    buf->set(5, 0xFF);
    auto slice = NamedBufferSlice::create("s", buf, 5, 1);
    
    auto copy = slice->clone();
    auto casted = std::dynamic_pointer_cast<NamedBufferSlice>(copy);
    ASSERT_NE(casted, nullptr);
    EXPECT_EQ(casted->get(0), 0xFF);
}
