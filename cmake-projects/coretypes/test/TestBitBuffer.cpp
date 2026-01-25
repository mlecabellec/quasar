#include "quasar/coretypes/BitBuffer.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace quasar::coretypes;

TEST(BitBufferTest, GetSetBit) {
  BitBuffer bb(16); // 2 bytes
  bb.setBit(0, true);
  bb.setBit(15, true);
  EXPECT_TRUE(bb.getBit(0));
  EXPECT_FALSE(bb.getBit(1));
  EXPECT_TRUE(bb.getBit(15));
}

TEST(BitBufferTest, BitSize) {
  BitBuffer bb(10); // 2 bytes allocated. 10 bits valid.
  EXPECT_EQ(bb.bitSize(), 10);
  EXPECT_EQ(bb.size(), 2);
}

TEST(BitBufferTest, SliceBits) {
  // 0xFO -> 1111 0000
  // Bits: 0-1-1-1-1-1-0-0
  BitBuffer bb(8);
  bb.set(0, 0xF0);

  // Slice middle 4 bits: index 2 length 4.
  // 11[11 00]00
  //     ^  ^
  //     bits 2,3,4,5.
  // 2:1, 3:1, 4:0, 5:0 -> 1100
  BitBuffer sliced = bb.sliceBits(2, 4);
  EXPECT_EQ(sliced.bitSize(), 4);

  // 1100 -> 0xC, formatted in byte: 1100 0000 (0xC0) because bits are MSB
  // aligned? Implementation uses MSB first packing. 1 -> bit 0 -> 0x80 1 -> bit
  // 1 -> 0x40 0 -> bit 2 0 -> bit 3 0x80 | 0x40 = 0xC0.
  EXPECT_EQ(sliced.get(0) & 0xF0, 0xC0);
}

TEST(BitBufferTest, ConcatBits) {
  // A: 11 (2 bits)
  // B: 00 (2 bits)
  // Concat -> 1100 (4 bits)
  BitBuffer a(2);
  a.setBit(0, true);
  a.setBit(1, true);

  BitBuffer b(2);
  b.setBit(0, false);
  b.setBit(1, false);

  BitBuffer c = a.concatBits(b);
  EXPECT_EQ(c.bitSize(), 4);
  EXPECT_TRUE(c.getBit(0));
  EXPECT_TRUE(c.getBit(1));
  EXPECT_FALSE(c.getBit(2));
  EXPECT_FALSE(c.getBit(3));
}

TEST(BitBufferTest, ReverseBits) {
  // 1100 -> 0011
  BitBuffer bb(4);
  bb.setBit(0, true);
  bb.setBit(1, true);
  bb.setBit(2, false);
  bb.setBit(3, false);

  bb.reverseBits();
  EXPECT_FALSE(bb.getBit(0));
  EXPECT_FALSE(bb.getBit(1));
  EXPECT_TRUE(bb.getBit(2));
  EXPECT_TRUE(bb.getBit(3));
}

TEST(BitBufferTest, ReverseBitsGroup) {
  // 6 bits: 10 11 00
  // Rev group 2: 00 11 10
  BitBuffer bb(6);
  bb.setBit(0, true);
  bb.setBit(1, false); // 10
  bb.setBit(2, true);
  bb.setBit(3, true); // 11
  bb.setBit(4, false);
  bb.setBit(5, false); // 00

  bb.reverseBits(2);
  // 00 -> 0,1
  EXPECT_FALSE(bb.getBit(0));
  EXPECT_FALSE(bb.getBit(1));
  // 11 -> 2,3
  EXPECT_TRUE(bb.getBit(2));
  EXPECT_TRUE(bb.getBit(3));
  // 10 -> 4,5
  EXPECT_TRUE(bb.getBit(4));
  EXPECT_FALSE(bb.getBit(5));
}
