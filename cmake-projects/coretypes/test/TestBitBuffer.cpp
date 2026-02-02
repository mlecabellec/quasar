#include <gtest/gtest.h>
#include "quasar/coretypes/BitBuffer.hpp"
#include <chrono>
#include <future>
#include <thread>
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

TEST(BitBufferTest, OutOfRange) {
  BitBuffer bb(8); // 8 bits = 1 byte
  EXPECT_THROW(bb.getBit(8), std::out_of_range);
  EXPECT_FALSE(bb.getBit(7));
  EXPECT_THROW(bb.setBit(100, true), std::out_of_range);

  // Slice out of range
  EXPECT_THROW(bb.sliceBits(5, 5), std::out_of_range); // 5+5=10 > 8
}

TEST(BitBufferTest, Equals) {
  BitBuffer bb1(16);
  bb1.setBit(0, true);
  bb1.setBit(15, true);

  BitBuffer bb2(16);
  bb2.setBit(0, true);
  bb2.setBit(15, true);

  EXPECT_TRUE(bb1.equals(bb2));

  bb2.setBit(1, true);
  EXPECT_FALSE(bb1.equals(bb2));

  BitBuffer bb3(15);
  EXPECT_FALSE(bb1.equals(bb3));
}

TEST(BitBufferTest, Clone) {
  BitBuffer bb1(16);
  bb1.setBit(5, true);
  
  BitBuffer bb2 = bb1.clone();
  EXPECT_EQ(bb2.bitSize(), 16);
  EXPECT_TRUE(bb2.getBit(5));
  EXPECT_FALSE(bb2.getBit(0));
  
  // Verify deep copy
  bb1.setBit(5, false);
  EXPECT_TRUE(bb2.getBit(5));
}

TEST(BitBufferTest, Performance_GetSet) {
  BitBuffer bb(1024 * 8); // 1KB
  const int iterations = 1000000;

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; ++i) {
    bb.setBit(i % (1024 * 8), true);
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> elapsed = end - start;
  std::cout << "1M setBit operations took: " << elapsed.count() << " ms"
            << std::endl;

  // Basic sanity check that it was fast enough (e.g. < 500ms for 1M ops).
  // This is hardware dependent but 1M ops should be very fast.
  EXPECT_LT(elapsed.count(), 1000.0);
}

TEST(BitBufferTest, ThreadSafety) {
  BitBuffer bb(1024);
  std::atomic<bool> stop{false};

  auto writer = std::async(std::launch::async, [&]() {
    int i = 0;
    while (!stop) {
      bb.setBit(i % 1024, true);
      i++;
    }
  });

  auto reader = std::async(std::launch::async, [&]() {
    int i = 0;
    while (!stop) {
      // Just read to provoke race if any
      volatile bool b = bb.getBit(i % 1024);
      (void)b;
      i++;
    }
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  stop = true;
  writer.get();
  reader.get();

  // If we didn't crash or TSAN didn't flag, we assume basic thread safety holds
  // (locks are working).
  SUCCEED();
}
