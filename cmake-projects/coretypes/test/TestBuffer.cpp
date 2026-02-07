#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BufferSlice.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <vector>

using namespace quasar::coretypes;

TEST(BufferTest, ConstructAndSize) {
  // Step: Initialize empty Buffer
  std::cout << "Step: Initialize empty Buffer" << std::endl;
  Buffer b1;

  // Assertion: Check if size is 0
  std::cout << "Assertion: Check if size is 0" << std::endl;
  EXPECT_EQ(b1.size(), 0);

  // Step: Initialize Buffer with size 10
  std::cout << "Step: Initialize Buffer with size 10" << std::endl;
  Buffer b2(10);

  // Assertion: Check if size is 10
  std::cout << "Assertion: Check if size is 10" << std::endl;
  EXPECT_EQ(b2.size(), 10);

  // Assertion: Check if first byte is 0
  std::cout << "Assertion: Check if first byte is 0" << std::endl;
  EXPECT_EQ(b2.get(0), 0);
}

TEST(BufferTest, SetGet) {
  // Step: Initialize Buffer with size 5
  std::cout << "Step: Initialize Buffer with size 5" << std::endl;
  Buffer b(5);

  // Step: Set index 0 to 0xAA
  std::cout << "Step: Set index 0 to 0xAA" << std::endl;
  b.set(0, 0xAA);

  // Assertion: Check if index 0 is 0xAA
  std::cout << "Assertion: Check if index 0 is 0xAA" << std::endl;
  EXPECT_EQ(b.get(0), 0xAA);

  // Assertion: Check if get(5) throws out_of_range
  std::cout << "Assertion: Check if get(5) throws out_of_range" << std::endl;
  EXPECT_THROW(b.get(5), std::out_of_range);

  // Assertion: Check if set(5) throws out_of_range
  std::cout << "Assertion: Check if set(5) throws out_of_range" << std::endl;
  EXPECT_THROW(b.set(5, 0), std::out_of_range);
}

TEST(BufferTest, ToString) {
  // Step: Initialize Buffer from vector
  std::cout << "Step: Initialize Buffer from vector {0xDE, 0xAD, 0xBE, 0xEF}"
            << std::endl;
  std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
  Buffer b(data);

  // Assertion: Check if toString matches "deadbeef"
  std::cout << "Assertion: Check if toString matches \"deadbeef\"" << std::endl;
  EXPECT_EQ(b.toString(), "deadbeef");
}

TEST(BufferTest, FromString) {
  // Step: Initialize Buffer from hex string "deadbeef"
  std::cout << "Step: Initialize Buffer from hex string \"deadbeef\""
            << std::endl;
  Buffer b = Buffer::fromString("deadbeef");

  // Assertion: Check size is 4
  std::cout << "Assertion: Check size is 4" << std::endl;
  EXPECT_EQ(b.size(), 4);

  // Assertion: Check first byte is 0xDE
  std::cout << "Assertion: Check first byte is 0xDE" << std::endl;
  EXPECT_EQ(b.get(0), 0xDE);

  // Assertion: Check last byte is 0xEF
  std::cout << "Assertion: Check last byte is 0xEF" << std::endl;
  EXPECT_EQ(b.get(3), 0xEF);
}

TEST(BufferTest, NumericIO) {
  // Step: Initialize Buffer with size 8
  std::cout << "Step: Initialize Buffer with size 8" << std::endl;
  Buffer b(8);

  // Step: Write 0x12345678 in BigEndian at index 0
  std::cout << "Step: Write 0x12345678 in BigEndian at index 0" << std::endl;
  b.writeInt(0x12345678, 0, Endianness::BigEndian);

  // Assertion: Check byte layout for BigEndian
  std::cout << "Assertion: Check first byte is 0x12" << std::endl;
  EXPECT_EQ(b.get(0), 0x12);
  std::cout << "Assertion: Check fourth byte is 0x78" << std::endl;
  EXPECT_EQ(b.get(3), 0x78);

  // Assertion: Check readInt BigEndian
  std::cout << "Assertion: Check readInt BigEndian returns 0x12345678"
            << std::endl;
  EXPECT_EQ(b.readInt(0, Endianness::BigEndian), 0x12345678);

  // Step: Write 0x12345678 in LittleEndian at index 4
  std::cout << "Step: Write 0x12345678 in LittleEndian at index 4" << std::endl;
  b.writeInt(0x12345678, 4, Endianness::LittleEndian);

  // Assertion: Check byte layout for LittleEndian
  std::cout << "Assertion: Check fifth byte is 0x78" << std::endl;
  EXPECT_EQ(b.get(4), 0x78);
  std::cout << "Assertion: Check eighth byte is 0x12" << std::endl;
  EXPECT_EQ(b.get(7), 0x12);

  // Assertion: Check readInt LittleEndian
  std::cout << "Assertion: Check readInt LittleEndian returns 0x12345678"
            << std::endl;
  EXPECT_EQ(b.readInt(4, Endianness::LittleEndian), 0x12345678);
}

TEST(BufferTest, SliceConcat) {
  // Step: Initialize b1 and b2 from strings
  std::cout << "Step: Initialize b1 and b2 from strings" << std::endl;
  Buffer b1 = Buffer::fromString("aabb");
  Buffer b2 = Buffer::fromString("ccdd");

  // Step: Concatenate b1 and b2
  std::cout << "Step: Concatenate b1 and b2" << std::endl;
  Buffer b3 = b1.concat(b2);

  // Assertion: Check concatenated string is "aabbccdd"
  std::cout << "Assertion: Check concatenated string is \"aabbccdd\""
            << std::endl;
  EXPECT_EQ(b3.toString(), "aabbccdd");

  // Step: Slice b3 from index 2 length 2
  std::cout << "Step: Slice b3 from index 2 length 2" << std::endl;
  Buffer b4 = b3.slice(2, 2);

  // Assertion: Check sliced string is "ccdd"
  std::cout << "Assertion: Check sliced string is \"ccdd\"" << std::endl;
  EXPECT_EQ(b4.toString(), "ccdd");
}

TEST(BufferTest, Reverse) {
  // Step: Initialize Buffer and reverse it
  std::cout << "Step: Initialize Buffer and reverse it" << std::endl;
  Buffer b = Buffer::fromString("01020304");
  b.reverse();

  // Assertion: Check reversed string is "04030201"
  std::cout << "Assertion: Check reversed string is \"04030201\"" << std::endl;
  EXPECT_EQ(b.toString(), "04030201");
}

TEST(BufferTest, ReverseWord) {
  // Step: Initialize Buffer and reverse in words of 4 bytes
  std::cout << "Step: Initialize Buffer and reverse in words of 4 bytes"
            << std::endl;
  Buffer b = Buffer::fromString("0102030405060708");
  b.reverse(4);

  // Assertion: Check reversed string is "0506070801020304"
  std::cout << "Assertion: Check reversed string is \"0506070801020304\""
            << std::endl;
  EXPECT_EQ(b.toString(), "0506070801020304");
}

TEST(BufferTest, Clone) {
  // Step: Initialize Buffer b and its clone b2
  std::cout << "Step: Initialize Buffer b and its clone b2" << std::endl;
  Buffer b = Buffer::fromString("aa");
  Buffer b2 = b.clone();

  // Assertion: Check if b initially equals b2
  std::cout << "Assertion: Check if b equals b2" << std::endl;
  EXPECT_TRUE(b.equals(b2));

  // Step: Modify b
  std::cout << "Step: Modify b" << std::endl;
  b.set(0, 0xbb);

  // Assertion: Check if b no longer equals b2
  std::cout << "Assertion: Check if b no longer equals b2" << std::endl;
  EXPECT_FALSE(b.equals(b2));

  // Assertion: Check if b2 is unchanged
  std::cout << "Assertion: Check if b2 value is still \"aa\"" << std::endl;
  EXPECT_EQ(b2.toString(), "aa");
}

// New Tests

TEST(BufferTest, SliceView) {
  // Step: Initialize shared Buffer and set content
  std::cout << "Step: Initialize shared Buffer and set content" << std::endl;
  auto b = std::make_shared<Buffer>(10);
  b->set(0, 0x11);
  b->set(1, 0x22);

  // Step: Create sliceView from index 0 length 5
  std::cout << "Step: Create sliceView from index 0 length 5" << std::endl;
  std::shared_ptr<BufferSlice> slice = b->sliceView(0, 5);

  // Assertion: Check slice size is 5
  std::cout << "Assertion: Check slice size is 5" << std::endl;
  EXPECT_EQ(slice->size(), 5);

  // Assertion: Check first byte in slice is 0x11
  std::cout << "Assertion: Check first byte in slice is 0x11" << std::endl;
  EXPECT_EQ(slice->get(0), 0x11);

  // Step: Modify original buffer
  std::cout << "Step: Modify original buffer at index 0" << std::endl;
  b->set(0, 0x33);

  // Assertion: Check if slice reflects changes in original buffer
  std::cout << "Assertion: Check if slice reflects 0x33 change" << std::endl;
  EXPECT_EQ(slice->get(0), 0x33); // Should reflect change

  // Step: Modify slice
  std::cout << "Step: Modify slice at index 1" << std::endl;
  slice->set(1, 0x44);

  // Assertion: Check if original buffer reflects changes in slice
  std::cout << "Assertion: Check if original buffer reflects 0x44 change"
            << std::endl;
  EXPECT_EQ(b->get(1), 0x44); // Should reflect change
}

TEST(BufferTest, BitwiseOps) {
  // Step: Initialize b1 and b2 for bitwise ops
  std::cout << "Step: Initialize b1 and b2 for bitwise ops" << std::endl;
  Buffer b1 = Buffer::fromString("f0");
  Buffer b2 = Buffer::fromString("0f");

  // Assertion: Check bitwiseAnd
  std::cout << "Assertion: Check bitwiseAnd(f0, 0f) is 00" << std::endl;
  Buffer band = b1.bitwiseAnd(b2);
  EXPECT_EQ(band.toString(), "00");

  // Assertion: Check bitwiseOr
  std::cout << "Assertion: Check bitwiseOr(f0, 0f) is ff" << std::endl;
  Buffer bor = b1.bitwiseOr(b2);
  EXPECT_EQ(bor.toString(), "ff");

  // Assertion: Check bitwiseXor
  std::cout << "Assertion: Check bitwiseXor(f0, 0f) is ff" << std::endl;
  Buffer bxor = b1.bitwiseXor(b2);
  EXPECT_EQ(bxor.toString(), "ff");

  // Assertion: Check bitwiseNot
  std::cout << "Assertion: Check bitwiseNot(f0) is 0f" << std::endl;
  Buffer bnot = b1.bitwiseNot();
  EXPECT_EQ(bnot.toString(), "0f");

  // Step: Check strict size requirements for bitwise ops
  std::cout << "Step: Check strict size requirements for bitwise ops"
            << std::endl;
  Buffer b4 = Buffer::fromString("0000"); // 2 bytes

  // Assertion: Check if bitwiseAnd(1 byte, 2 bytes) throws invalid_argument
  std::cout << "Assertion: Check if bitwiseAnd(1 byte, 2 bytes) throws "
               "invalid_argument"
            << std::endl;
  EXPECT_THROW(b1.bitwiseAnd(b4), std::invalid_argument);
}

TEST(BufferTest, ComparisonOps) {
  // Step: Initialize b1, b2, b3 for comparison
  std::cout << "Step: Initialize b1, b2, b3 for comparison" << std::endl;
  Buffer b1 = Buffer::fromString("aabb");
  Buffer b2 = Buffer::fromString("aabb");
  Buffer b3 = Buffer::fromString("aacc");

  // Assertion: Check if b1 compareTo b2 is 0
  std::cout << "Assertion: Check if b1 compareTo b2 is 0" << std::endl;
  EXPECT_EQ(b1.compareTo(b2), 0);

  // Assertion: Check if b1 compareTo b3 is less than 0
  std::cout << "Assertion: Check if b1 compareTo b3 is less than 0"
            << std::endl;
  EXPECT_LT(b1.compareTo(b3), 0);

  // Assertion: Check if b3 compareTo b1 is greater than 0
  std::cout << "Assertion: Check if b3 compareTo b1 is greater than 0"
            << std::endl;
  EXPECT_GT(b3.compareTo(b1), 0);

  // Step: Compare Buffer with vector
  std::cout << "Step: Compare Buffer with vector {0xaa, 0xbb}" << std::endl;
  std::vector<uint8_t> vec = {0xaa, 0xbb};

  // Assertion: Check if b1 equals vec
  std::cout << "Assertion: Check if b1 equals vec" << std::endl;
  EXPECT_TRUE(b1.equals(vec));
}
