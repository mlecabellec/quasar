#include "quasar/coretypes/FloatingPointTypes.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>

using namespace quasar::coretypes;

TEST(FloatingPointTest, Arithmetic) {
  // Step: Initialize Float objects 'a' and 'b'
  std::cout << "Step: Initialize Float objects 'a' and 'b'" << std::endl;
  Float a(10.5f);
  Float b(2.0f);

  // Assertion: Check if addition results in 12.5
  std::cout << "Assertion: Check if addition results in 12.5" << std::endl;
  EXPECT_FLOAT_EQ(a.add(b).toFloat(), 12.5f);

  // Assertion: Check if division results in 5.25
  std::cout << "Assertion: Check if division results in 5.25" << std::endl;
  EXPECT_FLOAT_EQ(a.divide(b).toFloat(), 5.25f);
}

TEST(FloatingPointTest, SafeArithmetic) {
  // Step: Initialize Double object with max value and another with 2.0
  std::cout
      << "Step: Initialize Double object with max value and another with 2.0"
      << std::endl;
  Double max(std::numeric_limits<double>::max());
  Double two(2.0);

  // Assertion: Check if safeMultiply(2.0) throws overflow_error
  std::cout << "Assertion: Check if safeMultiply(2.0) throws overflow_error"
            << std::endl;
  EXPECT_THROW(max.safeMultiply(two), std::overflow_error);

  // Step: Initialize Double objects for division by zero test
  std::cout << "Step: Initialize Double objects for division by zero test"
            << std::endl;
  Double zero(0.0);
  Double one(1.0);

  // Assertion: Check if safeDivide(0.0) throws runtime_error
  std::cout << "Assertion: Check if safeDivide(0.0) throws runtime_error"
            << std::endl;
  EXPECT_THROW(one.safeDivide(zero), std::runtime_error);

  // Step: Initialize Double object with NaN
  std::cout << "Step: Initialize Double object with NaN" << std::endl;
  Double nan(std::numeric_limits<double>::quiet_NaN());

  // Assertion: Check if safeAdd(NaN) throws runtime_error
  std::cout << "Assertion: Check if safeAdd(NaN) throws runtime_error"
            << std::endl;
  EXPECT_THROW(one.safeAdd(nan), std::runtime_error);
}

TEST(FloatingPointTest, StringParsing) {
  // Step: Parse Float from "3.14"
  std::cout << "Step: Parse Float from \"3.14\"" << std::endl;
  Float f("3.14");

  // Assertion: Check if parsed value is 3.14
  std::cout << "Assertion: Check if parsed value is 3.14" << std::endl;
  EXPECT_FLOAT_EQ(f.toFloat(), 3.14f);

  // Step: Parse Double from "1.5"
  std::cout << "Step: Parse Double from \"1.5\"" << std::endl;
  Double d("1.5");

  // Assertion: Check if parsed value is 1.5
  std::cout << "Assertion: Check if parsed value is 1.5" << std::endl;
  EXPECT_DOUBLE_EQ(d.toDouble(), 1.5);

  // Assertion: Check if parsing "invalid" throws invalid_argument
  std::cout << "Assertion: Check if parsing \"invalid\" throws invalid_argument"
            << std::endl;
  EXPECT_THROW(Float("invalid"), std::invalid_argument);

  // Assertion: Check if parsing "1.23xyz" throws invalid_argument
  std::cout << "Assertion: Check if parsing \"1.23xyz\" throws invalid_argument"
            << std::endl;
  EXPECT_THROW(Float("1.23xyz"), std::invalid_argument); // Strict check
}

TEST(FloatingPointTest, PrimitiveComparison) {
  // Step: Initialize Double object with 3.14
  std::cout << "Step: Initialize Double object with 3.14" << std::endl;
  Double d(3.14);

  // Assertion: Check if equals(3.14) is true
  std::cout << "Assertion: Check if equals(3.14) is true" << std::endl;
  EXPECT_TRUE(d.equals(3.14));

  // Assertion: Check if equals(3.15) is false
  std::cout << "Assertion: Check if equals(3.15) is false" << std::endl;
  EXPECT_FALSE(d.equals(3.15));

  // Assertion: Check compareTo(3.14) is 0
  std::cout << "Assertion: Check compareTo(3.14) is 0" << std::endl;
  EXPECT_EQ(d.compareTo(3.14), 0);

  // Assertion: Check compareTo(3.15) is less than 0
  std::cout << "Assertion: Check compareTo(3.15) is less than 0" << std::endl;
  EXPECT_LT(d.compareTo(3.15), 0);

  // Assertion: Check compareTo(3.13) is greater than 0
  std::cout << "Assertion: Check compareTo(3.13) is greater than 0"
            << std::endl;
  EXPECT_GT(d.compareTo(3.13), 0);

  // Assertion: Check operator== with 3.14
  std::cout << "Assertion: Check operator== with 3.14" << std::endl;
  EXPECT_TRUE(d == 3.14);

  // Assertion: Check operator!= with 3.15
  std::cout << "Assertion: Check operator!= with 3.15" << std::endl;
  EXPECT_TRUE(d != 3.15);

  // Assertion: Check operator< with 3.15
  std::cout << "Assertion: Check operator< with 3.15" << std::endl;
  EXPECT_TRUE(d < 3.15);

  // Assertion: Check operator> with 3.13
  std::cout << "Assertion: Check operator> with 3.13" << std::endl;
  EXPECT_TRUE(d > 3.13);

  // Step: Initialize Double object with NaN
  std::cout << "Step: Initialize Double object with NaN" << std::endl;
  Double nan(std::numeric_limits<double>::quiet_NaN());

  // Assertion: Check if NaN equals NaN
  std::cout << "Assertion: Check if NaN equals NaN" << std::endl;
  EXPECT_TRUE(nan.equals(std::numeric_limits<double>::quiet_NaN()));
}

TEST(FloatingPointTest, ToStringHex) {
  // Step: Initialize Float with 15.5
  std::cout << "Step: Initialize Float with 15.5" << std::endl;
  Float f(15.5f);

  // Step: Convert 15.5 to hex string
  std::cout << "Step: Convert 15.5 to hex string" << std::endl;
  std::string s = Float::toString(15.5f, 16);

  // Assertion: Check if string contains "0x"
  std::cout << "Assertion: Check if string contains \"0x\"" << std::endl;
  EXPECT_NE(s.find("0x"), std::string::npos);

  // Assertion: Check if string contains "p"
  std::cout << "Assertion: Check if string contains \"p\"" << std::endl;
  EXPECT_NE(s.find("p"), std::string::npos);
}
