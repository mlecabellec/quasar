#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBitBufferSlice.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBufferSlice.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;
using namespace quasar::coretypes;

TEST(NamedBufferSliceTest, CreationAndUsage) {
  // Proof of compliance: [FE-0030.7] Create a NamedBufferSlice class.
  // Proof of compliance: [FE-0030.7.5] Slices can be created from a Buffer.
  // Proof of compliance: [FE-0030.7.6] Slices can be created from a slice.
  // Step: Initialize Buffer and set values
  std::cout << "Step: Initialize Buffer and set values" << std::endl;
  std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(10);
  buf->set(0, 0xAA);
  buf->set(1, 0xBB);

  // Step: Create NamedBufferSlice
  std::cout << "Step: Create NamedBufferSlice \"slice1\" from index 0 length 5"
            << std::endl;
  std::shared_ptr<NamedBufferSlice> slice =
      NamedBufferSlice::create("slice1", buf, 0, 5);

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
  std::shared_ptr<NamedBufferSlice> subSlice = slice->sliceView(1, 2);

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
  // Proof of compliance: [FE-0030.7] Create a NamedBitBufferSlice class.
  // Proof of compliance: [FE-0030.7.5] Slices can be created from a BitBuffer.
  // Step: Initialize BitBuffer and set bits
  std::cout << "Step: Initialize BitBuffer and set bits" << std::endl;
  std::shared_ptr<BitBuffer> bb = std::make_shared<BitBuffer>(16);
  bb->setBit(0, true);
  bb->setBit(2, true);

  // Step: Create NamedBitBufferSlice
  std::cout
      << "Step: Create NamedBitBufferSlice \"bitslice1\" from index 0 length 8"
      << std::endl;
  std::shared_ptr<NamedBitBufferSlice> slice =
      NamedBitBufferSlice::create("bitslice1", bb, 0, 8);

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
  std::shared_ptr<NamedBitBufferSlice> subSlice = slice->sliceView(1, 4);

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
  // Proof of compliance: [FE-0030.7.2] A slice shall be able to be copied.
  // Step: Initialize Buffer and set value for cloning test
  std::cout << "Step: Initialize Buffer and set value for cloning test"
            << std::endl;
  std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(10);
  buf->set(5, 0xFF);

  // Step: Create NamedBufferSlice
  std::cout << "Step: Create NamedBufferSlice \"s\"" << std::endl;
  std::shared_ptr<NamedBufferSlice> slice =
      NamedBufferSlice::create("s", buf, 5, 1);

  // Step: Clone the slice
  std::cout << "Step: Clone the slice" << std::endl;
  std::shared_ptr<NamedObject> copy = slice->clone();

  // Step: Cast the copy back to NamedBufferSlice
  std::cout << "Step: Cast the copy back to NamedBufferSlice" << std::endl;
  std::shared_ptr<NamedBufferSlice> casted =
      std::dynamic_pointer_cast<NamedBufferSlice>(copy);

  // Assertion: Check if cast was successful
  std::cout << "Assertion: Check if cast was successful" << std::endl;
  ASSERT_NE(casted, nullptr);

  // Assertion: Check if cloned slice has the correct value
  std::cout << "Assertion: Check if cloned slice index 0 is 0xFF" << std::endl;
  EXPECT_EQ(casted->get(0), 0xFF);
}

TEST(NamedBufferSliceTest, OutOfBoundsTests) {
  // Proof of compliance: [TSK-20260301-001.6], [TSK-20260301-001.7] out of
  // bounds behavior
  std::cout << "Step: Initialize NamedBuffer for bounds testing" << std::endl;
  std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("buf", 10);

  std::cout << "Assertion: Over-extending slice should throw std::out_of_range"
            << std::endl;
  EXPECT_THROW(NamedBufferSlice::create("slice1", buf, 5, 6),
               std::out_of_range);
  EXPECT_THROW(NamedBufferSlice::create("slice1", buf, 15, 1),
               std::out_of_range);

  std::shared_ptr<NamedBufferSlice> validSlice =
      NamedBufferSlice::create("slice2", buf, 2, 5);
  std::cout
      << "Assertion: Over-extending sub-slice should throw std::out_of_range"
      << std::endl;
  EXPECT_THROW(NamedBufferSlice::create("subSlice1", validSlice, 3, 3),
               std::out_of_range);
  EXPECT_THROW(validSlice->sliceView(3, 3), std::out_of_range);
}

TEST(NamedBitBufferSliceTest, OutOfBoundsTests) {
  // Proof of compliance: [TSK-20260301-001.9] out of bounds behavior
  std::cout << "Step: Initialize NamedBitBuffer for bounds testing"
            << std::endl;
  std::shared_ptr<NamedBitBuffer> buf = NamedBitBuffer::create("bbuf", 10);

  std::cout
      << "Assertion: Over-extending bit slice should throw std::out_of_range"
      << std::endl;
  EXPECT_THROW(NamedBitBufferSlice::create("slice1", buf, 5, 6),
               std::out_of_range);
  EXPECT_THROW(NamedBitBufferSlice::create("slice1", buf, 15, 1),
               std::out_of_range);

  std::shared_ptr<NamedBitBufferSlice> validSlice =
      NamedBitBufferSlice::create("slice2", buf, 2, 5);
  std::cout
      << "Assertion: Over-extending sub-slice should throw std::out_of_range"
      << std::endl;
  EXPECT_THROW(NamedBitBufferSlice::create("subSlice1", validSlice, 3, 3),
               std::out_of_range);
  EXPECT_THROW(validSlice->sliceView(3, 3), std::out_of_range);
}

TEST(NamedBufferSliceTest, DeepCopyRebasing) {
  std::cout << "Step: Setup hierarchy with a root, Buffer, and Slices"
            << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("buf", 20, root);
  std::shared_ptr<NamedBufferSlice> slice1 =
      NamedBufferSlice::create("slice1", buf, 2, 10, buf);
  std::shared_ptr<NamedBufferSlice> slice2 =
      NamedBufferSlice::create("slice2", buf, 5, 5, root);
  std::shared_ptr<NamedBufferSlice> subSlice =
      NamedBufferSlice::create("subSlice", slice1, 3, 4, slice1);

  std::cout << "Step: Perform deep copy of the tree" << std::endl;
  auto clonedRoot = root->deepCopy();

  std::cout << "Step: Find nodes in cloned tree" << std::endl;
  auto clonedBuf =
      std::dynamic_pointer_cast<NamedBuffer>(clonedRoot->getFirstChild());
  auto clonedSlice2 =
      std::dynamic_pointer_cast<NamedBufferSlice>(clonedRoot->getLastChild());
  auto clonedSlice1 =
      std::dynamic_pointer_cast<NamedBufferSlice>(clonedBuf->getFirstChild());
  auto clonedSubSlice = std::dynamic_pointer_cast<NamedBufferSlice>(
      clonedSlice1->getFirstChild());

  std::cout << "Assertion: slice1 should rebase to clonedBuf" << std::endl;
  EXPECT_EQ(clonedSlice1->quasar::coretypes::BufferSlice::getParent(),
            clonedBuf);
  EXPECT_NE(clonedSlice1->quasar::coretypes::BufferSlice::getParent(), buf);

  std::cout << "Assertion: slice2 should NOT rebase, keeping original buf"
            << std::endl;
  EXPECT_EQ(clonedSlice2->quasar::coretypes::BufferSlice::getParent(), buf);

  std::cout
      << "Assertion: subSlice should rebase indirectly to clonedBuf via slice1"
      << std::endl;
  EXPECT_EQ(clonedSubSlice->quasar::coretypes::BufferSlice::getParent(),
            clonedBuf);
  EXPECT_EQ(clonedSubSlice->getOffset(), 5);

  // Break strong circular references created intentionally for rebasing tests
  slice1->setParent(nullptr);
  subSlice->setParent(nullptr);
  clonedSlice1->setParent(nullptr);
  clonedSubSlice->setParent(nullptr);
}

TEST(NamedBitBufferSliceTest, DeepCopyRebasing) {
  std::cout << "Step: Setup hierarchy with a root, BitBuffer, and Slices"
            << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedBitBuffer> buf =
      NamedBitBuffer::create("bbuf", 20, root);
  std::shared_ptr<NamedBitBufferSlice> slice1 =
      NamedBitBufferSlice::create("slice1", buf, 2, 10, buf);
  std::shared_ptr<NamedBitBufferSlice> slice2 =
      NamedBitBufferSlice::create("slice2", buf, 5, 5, root);
  std::shared_ptr<NamedBitBufferSlice> subSlice =
      NamedBitBufferSlice::create("subSlice", slice1, 3, 4, slice1);

  std::cout << "Step: Perform deep copy of the tree" << std::endl;
  auto clonedRoot = root->deepCopy();

  std::cout << "Step: Find nodes in cloned tree" << std::endl;
  auto clonedBuf =
      std::dynamic_pointer_cast<NamedBitBuffer>(clonedRoot->getFirstChild());
  auto clonedSlice2 = std::dynamic_pointer_cast<NamedBitBufferSlice>(
      clonedRoot->getLastChild());
  auto clonedSlice1 = std::dynamic_pointer_cast<NamedBitBufferSlice>(
      clonedBuf->getFirstChild());
  auto clonedSubSlice = std::dynamic_pointer_cast<NamedBitBufferSlice>(
      clonedSlice1->getFirstChild());

  std::cout << "Assertion: slice1 should rebase to clonedBuf" << std::endl;
  EXPECT_EQ(clonedSlice1->quasar::coretypes::BitBufferSlice::getParent(),
            clonedBuf);
  EXPECT_NE(clonedSlice1->quasar::coretypes::BitBufferSlice::getParent(), buf);

  std::cout << "Assertion: slice2 should NOT rebase, keeping original buf"
            << std::endl;
  EXPECT_EQ(clonedSlice2->quasar::coretypes::BitBufferSlice::getParent(), buf);

  std::cout
      << "Assertion: subSlice should rebase indirectly to clonedBuf via slice1"
      << std::endl;
  EXPECT_EQ(clonedSubSlice->quasar::coretypes::BitBufferSlice::getParent(),
            clonedBuf);
  EXPECT_EQ(clonedSubSlice->getOffset(), 5);

  // Break strong circular references created intentionally for rebasing tests
  slice1->setParent(nullptr);
  subSlice->setParent(nullptr);
  clonedSlice1->setParent(nullptr);
  clonedSubSlice->setParent(nullptr);
}
