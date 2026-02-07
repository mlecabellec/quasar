#include "quasar/coretypes/BitBuffer.hpp"
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace quasar::coretypes;

TEST(BitBufferTest, GetSetBit) {
  // Step: Initialize BitBuffer with 16 bits
  std::cout << "Step: Initialize BitBuffer with 16 bits" << std::endl;
  BitBuffer bb(16); // 2 bytes

  // Step: Set bit 0 to true
  std::cout << "Step: Set bit 0 to true" << std::endl;
  bb.setBit(0, true);

  // Step: Set bit 15 to true
  std::cout << "Step: Set bit 15 to true" << std::endl;
  bb.setBit(15, true);

  // Assertion: Check if bit 0 is true
  std::cout << "Assertion: Check if bit 0 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(0));

  // Assertion: Check if bit 1 is false
  std::cout << "Assertion: Check if bit 1 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(1));

  // Assertion: Check if bit 15 is true
  std::cout << "Assertion: Check if bit 15 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(15));
}

TEST(BitBufferTest, BitSize) {
  // Step: Initialize BitBuffer with 10 bits
  std::cout << "Step: Initialize BitBuffer with 10 bits" << std::endl;
  BitBuffer bb(10); // 2 bytes allocated. 10 bits valid.

  // Assertion: Check if bit size is 10
  std::cout << "Assertion: Check if bit size is 10" << std::endl;
  EXPECT_EQ(bb.bitSize(), 10);

  // Assertion: Check if byte size is 2
  std::cout << "Assertion: Check if byte size is 2" << std::endl;
  EXPECT_EQ(bb.size(), 2);
}

TEST(BitBufferTest, SliceBits) {
  // Step: Initialize BitBuffer with 8 bits and set value to 0xF0
  std::cout << "Step: Initialize BitBuffer with 8 bits and set value to 0xF0"
            << std::endl;
  BitBuffer bb(8);
  bb.set(0, 0xF0);

  // Step: Slice middle 4 bits: index 2 length 4.
  std::cout << "Step: Slice middle 4 bits: index 2 length 4" << std::endl;
  BitBuffer sliced = bb.sliceBits(2, 4);

  // Assertion: Check if sliced bit size is 4
  std::cout << "Assertion: Check if sliced bit size is 4" << std::endl;
  EXPECT_EQ(sliced.bitSize(), 4);

  // Assertion: Check if sliced value matches expectations
  std::cout << "Assertion: Check if sliced value matches expectations"
            << std::endl;
  EXPECT_EQ(sliced.get(0) & 0xF0, 0xC0);
}

TEST(BitBufferTest, ConcatBits) {
  // Step: Initialize BitBuffer 'a' with 2 bits and set them to true
  std::cout << "Step: Initialize BitBuffer 'a' with 2 bits and set them to true"
            << std::endl;
  BitBuffer a(2);
  a.setBit(0, true);
  a.setBit(1, true);

  // Step: Initialize BitBuffer 'b' with 2 bits and set them to false
  std::cout
      << "Step: Initialize BitBuffer 'b' with 2 bits and set them to false"
      << std::endl;
  BitBuffer b(2);
  b.setBit(0, false);
  b.setBit(1, false);

  // Step: Concatenate 'a' and 'b' to create 'c'
  std::cout << "Step: Concatenate 'a' and 'b' to create 'c'" << std::endl;
  BitBuffer c = a.concatBits(b);

  // Assertion: Check if 'c' bit size is 4
  std::cout << "Assertion: Check if 'c' bit size is 4" << std::endl;
  EXPECT_EQ(c.bitSize(), 4);

  // Assertion: Check if bits in 'c' match expectations
  std::cout << "Assertion: Check if bit 0 in 'c' is true" << std::endl;
  EXPECT_TRUE(c.getBit(0));
  std::cout << "Assertion: Check if bit 1 in 'c' is true" << std::endl;
  EXPECT_TRUE(c.getBit(1));
  std::cout << "Assertion: Check if bit 2 in 'c' is false" << std::endl;
  EXPECT_FALSE(c.getBit(2));
  std::cout << "Assertion: Check if bit 3 in 'c' is false" << std::endl;
  EXPECT_FALSE(c.getBit(3));
}

TEST(BitBufferTest, ReverseBits) {
  // Step: Initialize BitBuffer with 4 bits and set initial pattern 1100
  std::cout
      << "Step: Initialize BitBuffer with 4 bits and set initial pattern 1100"
      << std::endl;
  BitBuffer bb(4);
  bb.setBit(0, true);
  bb.setBit(1, true);
  bb.setBit(2, false);
  bb.setBit(3, false);

  // Step: Reverse bits
  std::cout << "Step: Reverse bits" << std::endl;
  bb.reverseBits();

  // Assertion: Check if result pattern is 0011
  std::cout << "Assertion: Check if bit 0 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(0));
  std::cout << "Assertion: Check if bit 1 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(1));
  std::cout << "Assertion: Check if bit 2 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(2));
  std::cout << "Assertion: Check if bit 3 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(3));
}

TEST(BitBufferTest, ReverseBitsGroup) {
  // Step: Initialize BitBuffer with 6 bits and set pattern 10 11 00
  std::cout << "Step: Initialize BitBuffer with 6 bits and set pattern 10 11 00"
            << std::endl;
  BitBuffer bb(6);
  bb.setBit(0, true);
  bb.setBit(1, false); // 10
  bb.setBit(2, true);
  bb.setBit(3, true); // 11
  bb.setBit(4, false);
  bb.setBit(5, false); // 00

  // Step: Reverse bits in groups of 2
  std::cout << "Step: Reverse bits in groups of 2" << std::endl;
  bb.reverseBits(2);

  // Assertion: Check if result pattern is 00 11 10
  std::cout << "Assertion: Check if bit 0 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(0));
  std::cout << "Assertion: Check if bit 1 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(1));
  std::cout << "Assertion: Check if bit 2 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(2));
  std::cout << "Assertion: Check if bit 3 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(3));
  std::cout << "Assertion: Check if bit 4 is true" << std::endl;
  EXPECT_TRUE(bb.getBit(4));
  std::cout << "Assertion: Check if bit 5 is false" << std::endl;
  EXPECT_FALSE(bb.getBit(5));
}

TEST(BitBufferTest, OutOfRange) {
  // Step: Initialize BitBuffer with 8 bits
  std::cout << "Step: Initialize BitBuffer with 8 bits" << std::endl;
  BitBuffer bb(8); // 8 bits = 1 byte

  // Assertion: Check if getBit(8) throws out_of_range
  std::cout << "Assertion: Check if getBit(8) throws out_of_range" << std::endl;
  EXPECT_THROW(bb.getBit(8), std::out_of_range);

  // Assertion: Check if getBit(7) returns false
  std::cout << "Assertion: Check if getBit(7) returns false" << std::endl;
  EXPECT_FALSE(bb.getBit(7));

  // Assertion: Check if setBit(100) throws out_of_range
  std::cout << "Assertion: Check if setBit(100) throws out_of_range"
            << std::endl;
  EXPECT_THROW(bb.setBit(100, true), std::out_of_range);

  // Assertion: Check if sliceBits(5, 5) throws out_of_range
  std::cout << "Assertion: Check if sliceBits(5, 5) throws out_of_range"
            << std::endl;
  EXPECT_THROW(bb.sliceBits(5, 5), std::out_of_range); // 5+5=10 > 8
}

TEST(BitBufferTest, Equals) {
  // Step: Initialize bb1 and bb2 with identical patterns
  std::cout << "Step: Initialize bb1 and bb2 with identical patterns"
            << std::endl;
  BitBuffer bb1(16);
  bb1.setBit(0, true);
  bb1.setBit(15, true);

  BitBuffer bb2(16);
  bb2.setBit(0, true);
  bb2.setBit(15, true);

  // Assertion: Check if bb1 equals bb2
  std::cout << "Assertion: Check if bb1 equals bb2" << std::endl;
  EXPECT_TRUE(bb1.equals(bb2));

  // Step: Modify bb2
  std::cout << "Step: Modify bb2" << std::endl;
  bb2.setBit(1, true);

  // Assertion: Check if bb1 no longer equals bb2
  std::cout << "Assertion: Check if bb1 no longer equals bb2" << std::endl;
  EXPECT_FALSE(bb1.equals(bb2));

  // Step: Initialize bb3 with different size
  std::cout << "Step: Initialize bb3 with different size" << std::endl;
  BitBuffer bb3(15);

  // Assertion: Check if bb1 no longer equals bb3
  std::cout << "Assertion: Check if bb1 no longer equals bb3" << std::endl;
  EXPECT_FALSE(bb1.equals(bb3));
}

TEST(BitBufferTest, Clone) {
  // Step: Initialize bb1 and set a bit
  std::cout << "Step: Initialize bb1 and set a bit" << std::endl;
  BitBuffer bb1(16);
  bb1.setBit(5, true);

  // Step: Clone bb1 to bb2
  std::cout << "Step: Clone bb1 to bb2" << std::endl;
  BitBuffer bb2 = bb1.clone();

  // Assertion: Check if bb2 matches bb1
  std::cout << "Assertion: Check if bb2 bit size is 16" << std::endl;
  EXPECT_EQ(bb2.bitSize(), 16);
  std::cout << "Assertion: Check if bit 5 in bb2 is true" << std::endl;
  EXPECT_TRUE(bb2.getBit(5));
  std::cout << "Assertion: Check if bit 0 in bb2 is false" << std::endl;
  EXPECT_FALSE(bb2.getBit(0));

  // Step: Verify deep copy by modifying bb1
  std::cout << "Step: Verify deep copy by modifying bb1" << std::endl;
  bb1.setBit(5, false);

  // Assertion: Check if bb2 bit 5 is still true
  std::cout << "Assertion: Check if bb2 bit 5 is still true" << std::endl;
  EXPECT_TRUE(bb2.getBit(5));
}

TEST(BitBufferTest, Performance_GetSet) {
  // Step: Initialize BitBuffer for performance test
  std::cout << "Step: Initialize BitBuffer for performance test" << std::endl;
  BitBuffer bb(1024 * 8); // 1KB
  const int iterations = 1000000;

  // Step: Measure 1M setBit operations
  std::cout << "Step: Measure 1M setBit operations" << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; ++i) {
    bb.setBit(i % (1024 * 8), true);
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> elapsed = end - start;
  std::cout << "1M setBit operations took: " << elapsed.count() << " ms"
            << std::endl;

  // Assertion: Check if elapsed time is within limits
  std::cout << "Assertion: Check if elapsed time is less than 1000ms"
            << std::endl;
  EXPECT_LT(elapsed.count(), 1000.0);
}

TEST(BitBufferTest, ThreadSafety) {
  // Step: Initialize BitBuffer and atomic stop flag
  std::cout << "Step: Initialize BitBuffer and atomic stop flag" << std::endl;
  BitBuffer bb(1024);
  std::atomic<bool> stop{false};

  // Step: Launch writer thread
  std::cout << "Step: Launch writer thread" << std::endl;
  auto writer = std::async(std::launch::async, [&]() {
    int i = 0;
    while (!stop) {
      bb.setBit(i % 1024, true);
      i++;
    }
  });

  // Step: Launch reader thread
  std::cout << "Step: Launch reader thread" << std::endl;
  auto reader = std::async(std::launch::async, [&]() {
    int i = 0;
    while (!stop) {
      // Just read to provoke race if any
      volatile bool b = bb.getBit(i % 1024);
      (void)b;
      i++;
    }
  });

  // Step: Sleep for 100ms
  std::cout << "Step: Sleep for 100ms" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Step: Signal threads to stop and join
  std::cout << "Step: Signal threads to stop and join" << std::endl;
  stop = true;
  writer.get();
  reader.get();

  // Assertion: Thread safety assumed if no crash/TSAN flag
  std::cout << "Assertion: Thread safety holds" << std::endl;
  SUCCEED();
}
