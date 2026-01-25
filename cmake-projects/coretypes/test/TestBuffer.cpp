#include "quasar/coretypes/Buffer.hpp"
#include <gtest/gtest.h>
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

  Buffer b4 = b3.slice(2, 2);       // Start 2, length 2 -> "cc"
  EXPECT_EQ(b4.toString(), "ccdd"); // Wait. 2+2=4.
                                    // Index 2, 3 -> cc.
  // string "aabbccdd": 0-aa, 1-bb -- wait hex string bytes.
  // aa=170, bb=187.
  // data: [0xaa, 0xbb, 0xcc, 0xdd]
  // index 0: aa
  // index 1: bb
  // index 2: cc
  // index 3: dd
  // slice(2, 2) -> start 2, len 2 -> [cc, dd].
  // toString -> "ccdd".
  // Correct.
}

TEST(BufferTest, Reverse) {
  Buffer b = Buffer::fromString("01020304");
  b.reverse();
  EXPECT_EQ(b.toString(), "04030201");
}

TEST(BufferTest, ReverseWord) {
  // 8 bytes: 01 02 03 04 05 06 07 08
  Buffer b = Buffer::fromString("0102030405060708");
  // Reverse with wordSize 4 -> 2 words.
  // [01020304] [05060708] -> [05060708] [01020304]
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
