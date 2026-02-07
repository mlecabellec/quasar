#include "quasar/coretypes/BitBuffer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using namespace quasar::coretypes;

TEST(BitBufferStress, LargeAllocation) {
  // Step: Calculate sizes for 100 MB buffer
  std::cout << "Step: Calculate sizes for 100 MB buffer" << std::endl;
  size_t sizeBytes = 100 * 1024 * 1024;
  size_t sizeBits = sizeBytes * 8;

  try {
    // Step: Initialize BitBuffer with large allocation
    std::cout << "Step: Initialize BitBuffer with large allocation"
              << std::endl;
    BitBuffer bb(sizeBits);

    // Assertion: Check if bit size matches expectations
    std::cout << "Assertion: Check if bit size matches expectations"
              << std::endl;
    EXPECT_EQ(bb.bitSize(), sizeBits);

    // Step: Set bit at the end
    std::cout << "Step: Set bit at the end" << std::endl;
    bb.setBit(sizeBits - 1, true);

    // Assertion: Check if bit at the end is true
    std::cout << "Assertion: Check if bit at the end is true" << std::endl;
    EXPECT_TRUE(bb.getBit(sizeBits - 1));

    // Assertion: Check if bit at the beginning is false
    std::cout << "Assertion: Check if bit at the beginning is false"
              << std::endl;
    EXPECT_FALSE(bb.getBit(0));
  } catch (const std::exception &e) {
    // Assertion: Allocation should not fail
    std::cout << "Assertion: Allocation should not fail" << std::endl;
    FAIL() << "Allocation failed: " << e.what();
  }
}

TEST(BitBufferStress, HeavySlice) {
  // Step: Initialize BitBuffer with 100,000 bits
  std::cout << "Step: Initialize BitBuffer with 100,000 bits" << std::endl;
  size_t bits = 100000;
  BitBuffer bb(bits);

  // Step: Set every other bit to true
  std::cout << "Step: Set every other bit to true" << std::endl;
  for (size_t i = 0; i < bits; i += 2) {
    bb.setBit(i, true);
  }

  // Step: Slice first half of the buffer
  std::cout << "Step: Slice first half of the buffer" << std::endl;
  BitBuffer sliced = bb.sliceBits(0, bits / 2);

  // Assertion: Check if sliced bit size is correct
  std::cout << "Assertion: Check if sliced bit size is correct" << std::endl;
  EXPECT_EQ(sliced.bitSize(), bits / 2);

  // Assertion: Check if bit 0 in sliced is true
  std::cout << "Assertion: Check if bit 0 in sliced is true" << std::endl;
  EXPECT_TRUE(sliced.getBit(0));

  // Assertion: Check if bit 1 in sliced is false
  std::cout << "Assertion: Check if bit 1 in sliced is false" << std::endl;
  EXPECT_FALSE(sliced.getBit(1));
}
