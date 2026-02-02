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

TEST(IntegerTest, PrimitiveInitialization) {
  // Signed types
  Byte b(static_cast<int8_t>(10));
  EXPECT_EQ(b.value(), 10);

  Short s(static_cast<int16_t>(1000));
  EXPECT_EQ(s.value(), 1000);

  Int i(100000);
  EXPECT_EQ(i.value(), 100000);

  Long l(10000000000L);
  EXPECT_EQ(l.value(), 10000000000L);

  // Unsigned types
  UByte ub(static_cast<uint8_t>(200));
  EXPECT_EQ(ub.value(), 200);

  UShort us(static_cast<uint16_t>(50000));
  EXPECT_EQ(us.value(), 50000);

  UInt ui(3000000000U);
  EXPECT_EQ(ui.value(), 3000000000U);

  ULong ul(10000000000000000000ULL);
  EXPECT_EQ(ul.value(), 10000000000000000000ULL);
}

TEST(IntegerTest, BitwiseOperations) {
  Int a(0b1010); // 10
  Int b(0b1100); // 12

  // AND
  // 1010 & 1100 = 1000 (8)
  EXPECT_EQ(a.bitwiseAnd(b)->toInt(), 8);

  // OR
  // 1010 | 1100 = 1110 (14)
  EXPECT_EQ(a.bitwiseOr(b)->toInt(), 14);

  // XOR
  // 1010 ^ 1100 = 0110 (6)
  EXPECT_EQ(a.bitwiseXor(b)->toInt(), 6);

  // NOT
  // ~0...00001010 = 1...11110101
  // For int32: ~10 = -11
  EXPECT_EQ(a.bitwiseNot()->toInt(), -11);

  // Shift Left
  // 10 << 1 = 20
  EXPECT_EQ(a.bitwiseLeftShift(1)->toInt(), 20);

  // Shift Right
  // 10 >> 1 = 5
  EXPECT_EQ(a.bitwiseRightShift(1)->toInt(), 5);
}

TEST(IntegerTest, Introspection) {
  Int i(42);
  EXPECT_EQ(i.getType(), "Integer");
  EXPECT_TRUE(i.isIntegerType());
  EXPECT_TRUE(i.isSigned());

  UByte ub(10);
  EXPECT_EQ(ub.getType(), "Integer");
  EXPECT_TRUE(ub.isIntegerType());
  EXPECT_FALSE(ub.isSigned());
}

TEST(IntegerTest, PrimitiveComparison) {
  Int i(42);
  EXPECT_TRUE(i.equals(42));
  EXPECT_FALSE(i.equals(43));
  
  EXPECT_EQ(i.compareTo(42), 0);
  EXPECT_LT(i.compareTo(43), 0);
  EXPECT_GT(i.compareTo(41), 0);
  
  EXPECT_TRUE(i == 42);
  EXPECT_TRUE(i != 43);
  EXPECT_TRUE(i < 43);
  EXPECT_TRUE(i > 41);
  EXPECT_TRUE(i <= 42);
  EXPECT_TRUE(i >= 42);
}
