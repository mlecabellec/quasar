#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BufferSlice.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace quasar::coretypes;

TEST(BufferTest, ConstructAndSize) {
  Buffer b1;
  EXPECT_EQ(b1.size(), 0);
  Buffer b2(10);
  EXPECT_EQ(b2.size(), 10);
  EXPECT_EQ(b2.get(0), 0);
}

TEST(BufferTest, SetGet) {
  Buffer b(5);
  b.set(0, 0xAA);
  EXPECT_EQ(b.get(0), 0xAA);
  EXPECT_THROW(b.get(5), std::out_of_range);
  EXPECT_THROW(b.set(5, 0), std::out_of_range);
}

TEST(BufferTest, ToString) {
  std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
  Buffer b(data);
  EXPECT_EQ(b.toString(), "deadbeef");
}

TEST(BufferTest, FromString) {
  Buffer b = Buffer::fromString("deadbeef");
  EXPECT_EQ(b.size(), 4);
  EXPECT_EQ(b.get(0), 0xDE);
  EXPECT_EQ(b.get(3), 0xEF);
}

TEST(BufferTest, NumericIO) {
  Buffer b(8);
  // Write BigEndian
  b.writeInt(0x12345678, 0, Endianness::BigEndian);
  EXPECT_EQ(b.get(0), 0x12);
  EXPECT_EQ(b.get(3), 0x78);
  EXPECT_EQ(b.readInt(0, Endianness::BigEndian), 0x12345678);

  // Write LittleEndian
  b.writeInt(0x12345678, 4, Endianness::LittleEndian);
  EXPECT_EQ(b.get(4), 0x78);
  EXPECT_EQ(b.get(7), 0x12);
  EXPECT_EQ(b.readInt(4, Endianness::LittleEndian), 0x12345678);
}

TEST(BufferTest, SliceConcat) {
  Buffer b1 = Buffer::fromString("aabb");
  Buffer b2 = Buffer::fromString("ccdd");

  Buffer b3 = b1.concat(b2);
  EXPECT_EQ(b3.toString(), "aabbccdd");

  Buffer b4 = b3.slice(2, 2);
  EXPECT_EQ(b4.toString(), "ccdd");
}

TEST(BufferTest, Reverse) {
  Buffer b = Buffer::fromString("01020304");
  b.reverse();
  EXPECT_EQ(b.toString(), "04030201");
}

TEST(BufferTest, ReverseWord) {
  Buffer b = Buffer::fromString("0102030405060708");
  b.reverse(4);
  EXPECT_EQ(b.toString(), "0506070801020304");
}

TEST(BufferTest, Clone) {
  Buffer b = Buffer::fromString("aa");
  Buffer b2 = b.clone();
  EXPECT_TRUE(b.equals(b2));
  b.set(0, 0xbb);
  EXPECT_FALSE(b.equals(b2));
  EXPECT_EQ(b2.toString(), "aa");
}

// New Tests

TEST(BufferTest, SliceView) {
  auto b = std::make_shared<Buffer>(10);
  b->set(0, 0x11);
  b->set(1, 0x22);

  std::shared_ptr<BufferSlice> slice = b->sliceView(0, 5);
  EXPECT_EQ(slice->size(), 5);
  EXPECT_EQ(slice->get(0), 0x11);

  // View semantics
  b->set(0, 0x33);
  EXPECT_EQ(slice->get(0), 0x33); // Should reflect change

  slice->set(1, 0x44);
  EXPECT_EQ(b->get(1), 0x44); // Should reflect change
}

TEST(BufferTest, BitwiseOps) {
  Buffer b1 = Buffer::fromString("f0");
  Buffer b2 = Buffer::fromString("0f");

  Buffer band = b1.bitwiseAnd(b2);
  EXPECT_EQ(band.toString(), "00");

  Buffer bor = b1.bitwiseOr(b2);
  EXPECT_EQ(bor.toString(), "ff");

  Buffer bxor = b1.bitwiseXor(b2);
  EXPECT_EQ(bxor.toString(), "ff");

  Buffer bnot = b1.bitwiseNot();
  EXPECT_EQ(bnot.toString(), "0f");

  Buffer b3 = Buffer::fromString("00"); // Different size check? No, 1 byte.
  // Check strictness
  Buffer b4 = Buffer::fromString("0000"); // 2 bytes
  EXPECT_THROW(b1.bitwiseAnd(b4), std::invalid_argument);
}

TEST(BufferTest, ComparisonOps) {
  Buffer b1 = Buffer::fromString("aabb");
  Buffer b2 = Buffer::fromString("aabb");
  Buffer b3 = Buffer::fromString("aacc");

  EXPECT_EQ(b1.compareTo(b2), 0);
  EXPECT_LT(b1.compareTo(b3), 0);
  EXPECT_GT(b3.compareTo(b1), 0);

  std::vector<uint8_t> vec = {0xaa, 0xbb};
  EXPECT_TRUE(b1.equals(vec));
}
