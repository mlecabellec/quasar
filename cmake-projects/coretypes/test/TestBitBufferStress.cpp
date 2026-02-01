#include "quasar/coretypes/BitBuffer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using namespace quasar::coretypes;

TEST(BitBufferStress, LargeAllocation) {
  // 100 MB buffer -> 800 Mbits
  size_t sizeBytes = 100 * 1024 * 1024;
  size_t sizeBits = sizeBytes * 8;

  try {
    BitBuffer bb(sizeBits);
    EXPECT_EQ(bb.bitSize(), sizeBits);

    // Set some bits at the end
    bb.setBit(sizeBits - 1, true);
    EXPECT_TRUE(bb.getBit(sizeBits - 1));
    EXPECT_FALSE(bb.getBit(0));
  } catch (const std::exception &e) {
    FAIL() << "Allocation failed: " << e.what();
  }
}

TEST(BitBufferStress, HeavySlice) {
  // Limit scope to avoid too long test
  size_t bits = 100000;
  BitBuffer bb(bits);
  for (size_t i = 0; i < bits; i += 2) {
    bb.setBit(i, true);
  }

  // Slice half
  BitBuffer sliced = bb.sliceBits(0, bits / 2);
  EXPECT_EQ(sliced.bitSize(), bits / 2);
  EXPECT_TRUE(sliced.getBit(0));
  EXPECT_FALSE(sliced.getBit(1));
}
