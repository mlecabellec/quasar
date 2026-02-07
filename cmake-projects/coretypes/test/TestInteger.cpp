#include "quasar/coretypes/IntegerTypes.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <limits>

using namespace quasar::coretypes;

TEST(IntegerTest, Arithmetic) {
  // Step: Initialize Integers 'a' and 'b'
  std::cout << "Step: Initialize Integers 'a' and 'b'" << std::endl;
  Int a(10);
  Int b(20);

  // Assertion: Check if addition results in 30
  std::cout << "Assertion: Check if addition results in 30" << std::endl;
  EXPECT_EQ(a.add(b).toInt(), 30);

  // Assertion: Check if subtraction results in -10
  std::cout << "Assertion: Check if subtraction results in -10" << std::endl;
  EXPECT_EQ(a.subtract(b).toInt(), -10);

  // Assertion: Check if multiplication results in 200
  std::cout << "Assertion: Check if multiplication results in 200" << std::endl;
  EXPECT_EQ(a.multiply(b).toInt(), 200);

  // Assertion: Check if division results in 2
  std::cout << "Assertion: Check if division results in 2" << std::endl;
  EXPECT_EQ(b.divide(a).toInt(), 2);
}

TEST(IntegerTest, SafeArithmeticOverflow) {
  // Step: Initialize max Int and one
  std::cout << "Step: Initialize max Int and one" << std::endl;
  Int max(std::numeric_limits<int32_t>::max());
  Int one(1);

  // Assertion: Check if safeAdd throws overflow_error
  std::cout << "Assertion: Check if safeAdd throws overflow_error" << std::endl;
  EXPECT_THROW(max.safeAdd(one), std::overflow_error);

  // Step: Initialize min Int
  std::cout << "Step: Initialize min Int" << std::endl;
  Int min(std::numeric_limits<int32_t>::min());

  // Assertion: Check if safeSubtract throws overflow_error
  std::cout << "Assertion: Check if safeSubtract throws overflow_error"
            << std::endl;
  EXPECT_THROW(min.safeSubtract(one), std::overflow_error);

  // Step: Initialize big Integers for multiplication
  std::cout << "Step: Initialize big Integers for multiplication" << std::endl;
  Int big(50000);
  Int big2(50000);

  // Assertion: Check if safeMultiply throws overflow_error
  std::cout << "Assertion: Check if safeMultiply throws overflow_error"
            << std::endl;
  EXPECT_THROW(big.safeMultiply(big2), std::overflow_error);
}

TEST(IntegerTest, DivisionByZero) {
  // Step: Initialize Int and zero
  std::cout << "Step: Initialize Int and zero" << std::endl;
  Int a(10);
  Int zero(0);

  // Assertion: Check if divide by zero throws runtime_error
  std::cout << "Assertion: Check if divide by zero throws runtime_error"
            << std::endl;
  EXPECT_THROW(a.divide(zero), std::runtime_error);

  // Assertion: Check if safeDivide by zero throws runtime_error
  std::cout << "Assertion: Check if safeDivide by zero throws runtime_error"
            << std::endl;
  EXPECT_THROW(a.safeDivide(zero), std::runtime_error);
}

TEST(IntegerTest, StringRadix) {
  // Step: Parse Int from "ff" base 16
  std::cout << "Step: Parse Int from \"ff\" base 16" << std::endl;
  Int i = Int::valueOf("ff", 16);

  // Assertion: Check if parsed value is 255
  std::cout << "Assertion: Check if parsed value is 255" << std::endl;
  EXPECT_EQ(i.toInt(), 255);

  // Assertion: Check if toString(255, 16) is "ff"
  std::cout << "Assertion: Check if toString(255, 16) is \"ff\"" << std::endl;
  EXPECT_EQ(Int::toString(255, 16), "ff");

  // Assertion: Check if toString(255, 2) is binary pattern
  std::cout << "Assertion: Check if toString(255, 2) is \"11111111\""
            << std::endl;
  EXPECT_EQ(Int::toString(255, 2), "11111111");
}

TEST(IntegerTest, ParseIntExceptions) {
  // Assertion: Check if parsing "foo" throws invalid_argument
  std::cout << "Assertion: Check if parsing \"foo\" throws invalid_argument"
            << std::endl;
  EXPECT_THROW(Int::parseInt("foo", 10), std::invalid_argument);

  // Assertion: Check if parsing "123a" throws invalid_argument
  std::cout << "Assertion: Check if parsing \"123a\" throws invalid_argument"
            << std::endl;
  EXPECT_THROW(Int::parseInt("123a", 10), std::invalid_argument);
}

TEST(IntegerTest, CompareTo) {
  // Step: Initialize Integers 'a' and 'b'
  std::cout << "Step: Initialize Integers 'a' and 'b'" << std::endl;
  Int a(5);
  Int b(10);

  // Assertion: Check if a < b
  std::cout << "Assertion: Check if a < b" << std::endl;
  EXPECT_LT(a.compareTo(b), 0);

  // Assertion: Check if b > a
  std::cout << "Assertion: Check if b > a" << std::endl;
  EXPECT_GT(b.compareTo(a), 0);

  // Assertion: Check if a == a
  std::cout << "Assertion: Check if a == a" << std::endl;
  EXPECT_EQ(a.compareTo(a), 0);

  // Assertion: Check if a equals 5
  std::cout << "Assertion: Check if a equals 5" << std::endl;
  EXPECT_TRUE(a.equals(Int(5)));
}

// New tests for templates
TEST(IntegerTest, LongType) {
  // Step: Initialize Longs 'l' and 'l2'
  std::cout << "Step: Initialize Longs 'l' and 'l2'" << std::endl;
  Long l(100L);
  Long l2(200L);

  // Assertion: Check if addition results in 300L
  std::cout << "Assertion: Check if addition results in 300L" << std::endl;
  EXPECT_EQ(l.add(l2).toLong(), 300L);

  // Step: Initialize max Long and one
  std::cout << "Step: Initialize max Long and one" << std::endl;
  Long max(std::numeric_limits<int64_t>::max());
  Long one(1);

  // Assertion: Check if safeAdd throws overflow_error
  std::cout << "Assertion: Check if safeAdd throws overflow_error" << std::endl;
  EXPECT_THROW(max.safeAdd(one), std::overflow_error);
}

TEST(IntegerTest, UByteType) {
  // Step: Initialize UBytes 'b1' and 'b2'
  std::cout << "Step: Initialize UBytes 'b1' and 'b2'" << std::endl;
  UByte b1(200);
  UByte b2(100);

  // Assertion: Check wrap-around addition
  std::cout << "Assertion: Check wrap-around addition (200 + 100 = 44)"
            << std::endl;
  EXPECT_EQ(b1.add(b2).toInt(), 44); // 300 % 256 = 44

  // Assertion: Check safeAdd throws overflow_error
  std::cout << "Assertion: Check safeAdd throws overflow_error" << std::endl;
  EXPECT_THROW(b1.safeAdd(b2), std::overflow_error);
}

TEST(IntegerTest, Endianness) {
  // Step: Initialize Int with 0x12345678 and swap bytes
  std::cout << "Step: Initialize Int with 0x12345678 and swap bytes"
            << std::endl;
  Int val(0x12345678);
  Int swapped = val.swapBytes();

  // Assertion: Check if value is swapped correctly
  std::cout << "Assertion: Check if value is 0x78563412" << std::endl;
  EXPECT_EQ(swapped.value(), 0x78563412);

  // Step: Initialize Short with 0x1234 and swap bytes
  std::cout << "Step: Initialize Short with 0x1234 and swap bytes" << std::endl;
  Short s(0x1234);
  Short sSwapped = s.swapBytes();

  // Assertion: Check if short is swapped correctly
  std::cout << "Assertion: Check if value is 0x3412" << std::endl;
  EXPECT_EQ(sSwapped.value(), 0x3412);
}

TEST(IntegerTest, PrimitiveInitialization) {
  // Step: Initialize signed types
  std::cout << "Step: Initialize signed types" << std::endl;

  // Assertion: Check Byte
  std::cout << "Assertion: Check Byte value is 10" << std::endl;
  Byte b(static_cast<int8_t>(10));
  EXPECT_EQ(b.value(), 10);

  // Assertion: Check Short
  std::cout << "Assertion: Check Short value is 1000" << std::endl;
  Short s(static_cast<int16_t>(1000));
  EXPECT_EQ(s.value(), 1000);

  // Assertion: Check Int
  std::cout << "Assertion: Check Int value is 100000" << std::endl;
  Int i(100000);
  EXPECT_EQ(i.value(), 100000);

  // Assertion: Check Long
  std::cout << "Assertion: Check Long value is 10000000000L" << std::endl;
  Long l(10000000000L);
  EXPECT_EQ(l.value(), 10000000000L);

  // Step: Initialize unsigned types
  std::cout << "Step: Initialize unsigned types" << std::endl;

  // Assertion: Check UByte
  std::cout << "Assertion: Check UByte value is 200" << std::endl;
  UByte ub(static_cast<uint8_t>(200));
  EXPECT_EQ(ub.value(), 200);

  // Assertion: Check UShort
  std::cout << "Assertion: Check UShort value is 50000" << std::endl;
  UShort us(static_cast<uint16_t>(50000));
  EXPECT_EQ(us.value(), 50000);

  // Assertion: Check UInt
  std::cout << "Assertion: Check UInt value is 3000000000U" << std::endl;
  UInt ui(3000000000U);
  EXPECT_EQ(ui.value(), 3000000000U);

  // Assertion: Check ULong
  std::cout << "Assertion: Check ULong value is 10000000000000000000ULL"
            << std::endl;
  ULong ul(10000000000000000000ULL);
  EXPECT_EQ(ul.value(), 10000000000000000000ULL);
}

TEST(IntegerTest, BitwiseOperations) {
  // Step: Initialize Integers 'a' (10) and 'b' (12)
  std::cout << "Step: Initialize Integers 'a' (10) and 'b' (12)" << std::endl;
  Int a(0b1010); // 10
  Int b(0b1100); // 12

  // Assertion: Check bitwiseAnd
  std::cout << "Assertion: Check bitwiseAnd(10, 12) is 8" << std::endl;
  EXPECT_EQ(a.bitwiseAnd(b)->toInt(), 8);

  // Assertion: Check bitwiseOr
  std::cout << "Assertion: Check bitwiseOr(10, 12) is 14" << std::endl;
  EXPECT_EQ(a.bitwiseOr(b)->toInt(), 14);

  // Assertion: Check bitwiseXor
  std::cout << "Assertion: Check bitwiseXor(10, 12) is 6" << std::endl;
  EXPECT_EQ(a.bitwiseXor(b)->toInt(), 6);

  // Assertion: Check bitwiseNot
  std::cout << "Assertion: Check bitwiseNot(10) is -11" << std::endl;
  EXPECT_EQ(a.bitwiseNot()->toInt(), -11);

  // Assertion: Check bitwiseLeftShift
  std::cout << "Assertion: Check bitwiseLeftShift(10, 1) is 20" << std::endl;
  EXPECT_EQ(a.bitwiseLeftShift(1)->toInt(), 20);

  // Assertion: Check bitwiseRightShift
  std::cout << "Assertion: Check bitwiseRightShift(10, 1) is 5" << std::endl;
  EXPECT_EQ(a.bitwiseRightShift(1)->toInt(), 5);
}

TEST(IntegerTest, Introspection) {
  // Step: Introspect signed Int
  std::cout << "Step: Introspect signed Int" << std::endl;
  Int i(42);

  // Assertion: Check type is "Integer"
  std::cout << "Assertion: Check type is \"Integer\"" << std::endl;
  EXPECT_EQ(i.getType(), "Integer");

  // Assertion: Check isIntegerType is true
  std::cout << "Assertion: Check isIntegerType is true" << std::endl;
  EXPECT_TRUE(i.isIntegerType());

  // Assertion: Check isSigned is true
  std::cout << "Assertion: Check isSigned is true" << std::endl;
  EXPECT_TRUE(i.isSigned());

  // Step: Introspect unsigned UByte
  std::cout << "Step: Introspect unsigned UByte" << std::endl;
  UByte ub(10);

  // Assertion: Check type is \"Integer\"
  std::cout << "Assertion: Check type is \"Integer\"" << std::endl;
  EXPECT_EQ(ub.getType(), "Integer");

  // Assertion: Check isIntegerType is true
  std::cout << "Assertion: Check isIntegerType is true" << std::endl;
  EXPECT_TRUE(ub.isIntegerType());

  // Assertion: Check isSigned is false
  std::cout << "Assertion: Check isSigned is false" << std::endl;
  EXPECT_FALSE(ub.isSigned());
}

TEST(IntegerTest, PrimitiveComparison) {
  // Step: Initialize Int with 42
  std::cout << "Step: Initialize Int with 42" << std::endl;
  Int i(42);

  // Assertion: Check equals(42) is true
  std::cout << "Assertion: Check equals(42) is true" << std::endl;
  EXPECT_TRUE(i.equals(42));

  // Assertion: Check equals(43) is false
  std::cout << "Assertion: Check equals(43) is false" << std::endl;
  EXPECT_FALSE(i.equals(43));

  // Assertion: Check compareTo(42) is 0
  std::cout << "Assertion: Check compareTo(42) is 0" << std::endl;
  EXPECT_EQ(i.compareTo(42), 0);

  // Assertion: Check compareTo(43) is less than 0
  std::cout << "Assertion: Check compareTo(43) is less than 0" << std::endl;
  EXPECT_LT(i.compareTo(43), 0);

  // Assertion: Check compareTo(41) is greater than 0
  std::cout << "Assertion: Check compareTo(41) is greater than 0" << std::endl;
  EXPECT_GT(i.compareTo(41), 0);

  // Assertion: Check operator== with 42
  std::cout << "Assertion: Check operator== with 42" << std::endl;
  EXPECT_TRUE(i == 42);

  // Assertion: Check operator!= with 43
  std::cout << "Assertion: Check operator!= with 43" << std::endl;
  EXPECT_TRUE(i != 43);

  // Assertion: Check operator< with 43
  std::cout << "Assertion: Check operator< with 43" << std::endl;
  EXPECT_TRUE(i < 43);

  // Assertion: Check operator> with 41
  std::cout << "Assertion: Check operator> with 41" << std::endl;
  EXPECT_TRUE(i > 41);

  // Assertion: Check operator<= with 42
  std::cout << "Assertion: Check operator<= with 42" << std::endl;
  EXPECT_TRUE(i <= 42);

  // Assertion: Check operator>= with 42
  std::cout << "Assertion: Check operator>= with 42" << std::endl;
  EXPECT_TRUE(i >= 42);
}
