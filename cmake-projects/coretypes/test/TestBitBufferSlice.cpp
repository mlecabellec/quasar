#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>

using namespace quasar::coretypes;

TEST(BitBufferSliceTest, CreationAndGet) {
  // Step: Initialize BitBuffer with 16 bits
  std::cout << "Step: Initialize BitBuffer with 16 bits" << std::endl;
  auto bb = std::make_shared<BitBuffer>(16); // 16 bits, 2 bytes

  // Step: Set bits index 0 and 15 to true
  std::cout << "Step: Set bits index 0 and 15 to true" << std::endl;
  bb->setBit(0, true);
  bb->setBit(15, true);

  // Step: Create slice of the entire buffer
  std::cout << "Step: Create slice of the entire buffer" << std::endl;
  auto slice = bb->sliceBitsView(0, 16);

  // Assertion: Check slice size is 16
  std::cout << "Assertion: Check slice size is 16" << std::endl;
  EXPECT_EQ(slice->size(), 16);

  // Assertion: Check if slice bits match original buffer
  std::cout << "Assertion: Check if slice bit 0 is true" << std::endl;
  EXPECT_TRUE(slice->getBit(0));
  std::cout << "Assertion: Check if slice bit 15 is true" << std::endl;
  EXPECT_TRUE(slice->getBit(15));
  std::cout << "Assertion: Check if slice bit 1 is false" << std::endl;
  EXPECT_FALSE(slice->getBit(1));
}

TEST(BitBufferSliceTest, SubSlice) {
  // Step: Initialize BitBuffer and set bit index 2 to true
  std::cout << "Step: Initialize BitBuffer and set bit index 2 to true"
            << std::endl;
  auto bb = std::make_shared<BitBuffer>(16);
  bb->setBit(2, true);

  // Step: Create slice from index 1 length 5
  std::cout << "Step: Create slice from index 1 length 5" << std::endl;
  auto slice = bb->sliceBitsView(1, 5); // bits 1..5

  // Assertion: Check slice size is 5
  std::cout << "Assertion: Check slice size is 5" << std::endl;
  EXPECT_EQ(slice->size(), 5);

  // Assertion: Check if bit 1 in slice (index 2 in buffer) is true
  std::cout << "Assertion: Check if bit 1 in slice is true" << std::endl;
  EXPECT_TRUE(slice->getBit(1));

  // Assertion: Check if bit 0 in slice (index 1 in buffer) is false
  std::cout << "Assertion: Check if bit 0 in slice is false" << std::endl;
  EXPECT_FALSE(slice->getBit(0));

  // Step: Modify bit 0 in slice and check reflection in buffer
  std::cout << "Step: Modify bit 0 in slice and check reflection in buffer"
            << std::endl;
  slice->setBit(0, true); // bb idx 1 becomes true

  // Assertion: Check if buffer index 1 is now true
  std::cout << "Assertion: Check if buffer index 1 is now true" << std::endl;
  EXPECT_TRUE(bb->getBit(1));
}

TEST(BitBufferSliceTest, Concat) {
  // Step: Initialize bb1 with bit 0 set to true
  std::cout << "Step: Initialize bb1 with bit 0 set to true" << std::endl;
  auto bb1 = std::make_shared<BitBuffer>(4);
  bb1->setBit(0, true); // 1000

  // Step: Initialize bb2 with bit 3 set to true
  std::cout << "Step: Initialize bb2 with bit 3 set to true" << std::endl;
  auto bb2 = std::make_shared<BitBuffer>(4);
  bb2->setBit(3, true); // 0001

  // Step: Create slices from bb1 and bb2
  std::cout << "Step: Create slices from bb1 and bb2" << std::endl;
  auto s1 = bb1->sliceBitsView(0, 4);
  auto s2 = bb2->sliceBitsView(0, 4);

  // Step: Concatenate slices s1 and s2
  std::cout << "Step: Concatenate slices s1 and s2" << std::endl;
  auto concat = s1->concat(*s2);

  // Assertion: Check concatenated bit size is 8
  std::cout << "Assertion: Check concatenated bit size is 8" << std::endl;
  EXPECT_EQ(concat->bitSize(), 8);

  // Assertion: Check bits in concatenated buffer
  std::cout << "Assertion: Check if bit 0 is true" << std::endl;
  EXPECT_TRUE(concat->getBit(0));
  std::cout << "Assertion: Check if bit 7 is true" << std::endl;
  EXPECT_TRUE(concat->getBit(7));
  std::cout << "Assertion: Check if bit 1 is false" << std::endl;
  EXPECT_FALSE(concat->getBit(1));
}
