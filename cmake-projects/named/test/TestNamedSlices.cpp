#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedBitBufferSlice.hpp"
#include "quasar/named/NamedBufferSlice.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;
using namespace quasar::coretypes;

TEST(NamedBufferSliceTest, CreationAndUsage) {
  // Step: Initialize Buffer and set values
  std::cout << "Step: Initialize Buffer and set values" << std::endl;
  auto buf = std::make_shared<Buffer>(10);
  buf->set(0, 0xAA);
  buf->set(1, 0xBB);

  // Step: Create NamedBufferSlice
  std::cout << "Step: Create NamedBufferSlice \"slice1\" from index 0 length 5"
            << std::endl;
  auto slice = NamedBufferSlice::create("slice1", buf, 0, 5);

  // Assertion: Check slice name
  std::cout << "Assertion: Check slice name is \"slice1\"" << std::endl;
  EXPECT_EQ(slice->getName(), "slice1");

  // Assertion: Check slice size
  std::cout << "Assertion: Check slice size is 5" << std::endl;
  EXPECT_EQ(slice->size(), 5);

  // Assertion: Check slice value at index 0
  std::cout << "Assertion: Check slice index 0 is 0xAA" << std::endl;
  EXPECT_EQ(slice->get(0), 0xAA);

  // Step: Create sub-slice view
  std::cout << "Step: Create sub-slice view from index 1 length 2" << std::endl;
  auto subSlice = slice->sliceView(1, 2);

  // Assertion: Check sub-slice name
  std::cout << "Assertion: Check sub-slice name is \"slice1_slice\""
            << std::endl;
  EXPECT_EQ(subSlice->getName(), "slice1_slice");

  // Assertion: Check sub-slice size
  std::cout << "Assertion: Check sub-slice size is 2" << std::endl;
  EXPECT_EQ(subSlice->size(), 2);

  // Assertion: Check sub-slice value at index 0 (0xBB from original buffer)
  std::cout << "Assertion: Check sub-slice index 0 is 0xBB" << std::endl;
  EXPECT_EQ(subSlice->get(0), 0xBB);
}

TEST(NamedBitBufferSliceTest, CreationAndUsage) {
  // Step: Initialize BitBuffer and set bits
  std::cout << "Step: Initialize BitBuffer and set bits" << std::endl;
  auto bb = std::make_shared<BitBuffer>(16);
  bb->setBit(0, true);
  bb->setBit(2, true);

  // Step: Create NamedBitBufferSlice
  std::cout
      << "Step: Create NamedBitBufferSlice \"bitslice1\" from index 0 length 8"
      << std::endl;
  auto slice = NamedBitBufferSlice::create("bitslice1", bb, 0, 8);

  // Assertion: Check slice name
  std::cout << "Assertion: Check slice name is \"bitslice1\"" << std::endl;
  EXPECT_EQ(slice->getName(), "bitslice1");

  // Assertion: Check slice size
  std::cout << "Assertion: Check slice size is 8" << std::endl;
  EXPECT_EQ(slice->size(), 8);

  // Assertion: Check slice bits
  std::cout << "Assertion: Check if slice bit 0 is true" << std::endl;
  EXPECT_TRUE(slice->getBit(0));
  std::cout << "Assertion: Check if slice bit 1 is false" << std::endl;
  EXPECT_FALSE(slice->getBit(1));
  std::cout << "Assertion: Check if slice bit 2 is true" << std::endl;
  EXPECT_TRUE(slice->getBit(2));

  // Step: Create sub-slice view from bit slice
  std::cout
      << "Step: Create sub-slice view from bit slice from index 1 length 4"
      << std::endl;
  auto subSlice = slice->sliceView(1, 4);

  // Assertion: Check sub-slice name
  std::cout << "Assertion: Check sub-slice name is \"bitslice1_slice\""
            << std::endl;
  EXPECT_EQ(subSlice->getName(), "bitslice1_slice");

  // Assertion: Check sub-slice size
  std::cout << "Assertion: Check sub-slice size is 4" << std::endl;
  EXPECT_EQ(subSlice->size(), 4);

  // Assertion: Check sub-slice bits
  std::cout << "Assertion: Check if sub-slice bit 0 is false (original bit 1)"
            << std::endl;
  EXPECT_FALSE(subSlice->getBit(0)); // bit 1 of original
  std::cout << "Assertion: Check if sub-slice bit 1 is true (original bit 2)"
            << std::endl;
  EXPECT_TRUE(subSlice->getBit(1)); // bit 2 of original
}

TEST(NamedBufferSliceTest, Clone) {
  // Step: Initialize Buffer and set value for cloning test
  std::cout << "Step: Initialize Buffer and set value for cloning test"
            << std::endl;
  auto buf = std::make_shared<Buffer>(10);
  buf->set(5, 0xFF);

  // Step: Create NamedBufferSlice
  std::cout << "Step: Create NamedBufferSlice \"s\"" << std::endl;
  auto slice = NamedBufferSlice::create("s", buf, 5, 1);

  // Step: Clone the slice
  std::cout << "Step: Clone the slice" << std::endl;
  auto copy = slice->clone();

  // Step: Cast the copy back to NamedBufferSlice
  std::cout << "Step: Cast the copy back to NamedBufferSlice" << std::endl;
  auto casted = std::dynamic_pointer_cast<NamedBufferSlice>(copy);

  // Assertion: Check if cast was successful
  std::cout << "Assertion: Check if cast was successful" << std::endl;
  ASSERT_NE(casted, nullptr);

  // Assertion: Check if cloned slice has the correct value
  std::cout << "Assertion: Check if cloned slice index 0 is 0xFF" << std::endl;
  EXPECT_EQ(casted->get(0), 0xFF);
}
