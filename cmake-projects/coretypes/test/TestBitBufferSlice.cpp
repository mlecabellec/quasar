#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace quasar::coretypes;

TEST(BitBufferSliceTest, CreationAndGet) {
  auto bb = std::make_shared<BitBuffer>(16); // 16 bits, 2 bytes
  bb->setBit(0, true);
  bb->setBit(15, true);
  // 1000.... ...1

  auto slice = bb->sliceBitsView(0, 16);
  EXPECT_EQ(slice->size(), 16);
  EXPECT_TRUE(slice->getBit(0));
  EXPECT_TRUE(slice->getBit(15));
  EXPECT_FALSE(slice->getBit(1));
}

TEST(BitBufferSliceTest, SubSlice) {
  auto bb = std::make_shared<BitBuffer>(16);
  bb->setBit(2, true);

  auto slice = bb->sliceBitsView(1, 5); // bits 1..5
  // slice idx 0 -> bb idx 1
  // slice idx 1 -> bb idx 2 (true)
  EXPECT_EQ(slice->size(), 5);
  EXPECT_TRUE(slice->getBit(1));
  EXPECT_FALSE(slice->getBit(0));

  // Modification reflected
  slice->setBit(0, true); // bb idx 1 becomes true
  EXPECT_TRUE(bb->getBit(1));
}

TEST(BitBufferSliceTest, Concat) {
  auto bb1 = std::make_shared<BitBuffer>(4);
  bb1->setBit(0, true); // 1000

  auto bb2 = std::make_shared<BitBuffer>(4);
  bb2->setBit(3, true); // 0001

  auto s1 = bb1->sliceBitsView(0, 4);
  auto s2 = bb2->sliceBitsView(0, 4);

  auto concat = s1->concat(*s2);
  // 1000 0001
  EXPECT_EQ(concat->bitSize(), 8);
  EXPECT_TRUE(concat->getBit(0));
  EXPECT_TRUE(concat->getBit(7));
  EXPECT_FALSE(concat->getBit(1));
}
