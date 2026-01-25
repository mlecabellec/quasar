#include "quasar/coretypes/IntegerTypes.hpp"
#include <gtest/gtest.h>
#include <limits>

using namespace quasar::coretypes;

TEST(IntegerTest, Arithmetic) {
  Int a(10);
  Int b(20);
  EXPECT_EQ(a.add(b).toInt(), 30);
  EXPECT_EQ(a.subtract(b).toInt(), -10);
  EXPECT_EQ(a.multiply(b).toInt(), 200);
  EXPECT_EQ(b.divide(a).toInt(), 2);
}

TEST(IntegerTest, SafeArithmeticOverflow) {
  Int max(std::numeric_limits<int32_t>::max());
  Int one(1);
  EXPECT_THROW(max.safeAdd(one), std::overflow_error);

  Int min(std::numeric_limits<int32_t>::min());
  EXPECT_THROW(min.safeSubtract(one), std::overflow_error);

  Int big(50000);
  Int big2(50000);
  EXPECT_THROW(big.safeMultiply(big2), std::overflow_error);
}

TEST(IntegerTest, DivisionByZero) {
  Int a(10);
  Int zero(0);
  EXPECT_THROW(a.divide(zero), std::runtime_error);
  EXPECT_THROW(a.safeDivide(zero), std::runtime_error);
}

TEST(IntegerTest, StringRadix) {
  Int i = Int::valueOf("ff", 16);
  EXPECT_EQ(i.toInt(), 255);
  EXPECT_EQ(Int::toString(255, 16), "ff");
  EXPECT_EQ(Int::toString(255, 2), "11111111");
}

TEST(IntegerTest, ParseIntExceptions) {
  EXPECT_THROW(Int::parseInt("foo", 10), std::invalid_argument);
  EXPECT_THROW(Int::parseInt("123a", 10), std::invalid_argument);
}

TEST(IntegerTest, CompareTo) {
  Int a(5);
  Int b(10);
  EXPECT_LT(a.compareTo(b), 0);
  EXPECT_GT(b.compareTo(a), 0);
  EXPECT_EQ(a.compareTo(a), 0);
  EXPECT_TRUE(a.equals(Int(5)));
}

// New tests for templates
TEST(IntegerTest, LongType) {
  Long l(100L);
  Long l2(200L);
  EXPECT_EQ(l.add(l2).toLong(), 300L);

  Long max(std::numeric_limits<int64_t>::max());
  Long one(1);
  EXPECT_THROW(max.safeAdd(one), std::overflow_error);
}

TEST(IntegerTest, UByteType) {
  UByte b1(200);
  UByte b2(100);
  // 200 + 100 = 300 -> overflow for uint8 (max 255)
  // Regular add wraps around
  EXPECT_EQ(b1.add(b2).toInt(), 44); // 300 % 256 = 44

  // Safe add throws
  EXPECT_THROW(b1.safeAdd(b2), std::overflow_error);
}

TEST(IntegerTest, Endianness) {
  // 0x12345678 -> swap -> 0x78563412
  Int val(0x12345678);
  Int swapped = val.swapBytes();
  EXPECT_EQ(swapped.value(), 0x78563412);

  // 0x1234 -> swap -> 0x3412
  Short s(0x1234);
  Short sSwapped = s.swapBytes();
  EXPECT_EQ(sSwapped.value(), 0x3412);
}
