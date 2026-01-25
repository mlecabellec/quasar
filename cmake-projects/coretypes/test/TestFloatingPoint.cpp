#include "quasar/coretypes/FloatingPointTypes.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <limits>

using namespace quasar::coretypes;

TEST(FloatingPointTest, Arithmetic) {
  Float a(10.5f);
  Float b(2.0f);
  EXPECT_FLOAT_EQ(a.add(b).toFloat(), 12.5f);
  EXPECT_FLOAT_EQ(a.divide(b).toFloat(), 5.25f);
}

TEST(FloatingPointTest, SafeArithmetic) {
  Double max(std::numeric_limits<double>::max());
  Double two(2.0);
  // max * 2 -> Infinity
  EXPECT_THROW(max.safeMultiply(two), std::overflow_error);

  Double zero(0.0);
  Double one(1.0);
  EXPECT_THROW(one.safeDivide(zero), std::runtime_error);

  // NaN
  Double nan(std::numeric_limits<double>::quiet_NaN());
  EXPECT_THROW(one.safeAdd(nan), std::runtime_error);
}

TEST(FloatingPointTest, StringParsing) {
  Float f("3.14");
  EXPECT_FLOAT_EQ(f.toFloat(), 3.14f);

  // Hex parsing
  // "0x1.fp3" = 1.9375 * 2^3 = 1.9375 * 8 = 15.5
  // std::stof handles hex if string starts with 0x?
  // Let's verify standard behavior or if manual handling needed.
  // C++17 stof supports hex floating point.
  // However, C++ streams hexfloat is output only? No, parsing too.
  // Let's test standard parsing first.
  Double d("1.5");
  EXPECT_DOUBLE_EQ(d.toDouble(), 1.5);

  EXPECT_THROW(Float("invalid"), std::invalid_argument);
  EXPECT_THROW(Float("1.23xyz"), std::invalid_argument); // Strict check
}

TEST(FloatingPointTest, ToStringHex) {
  Float f(15.5f);
  std::string s = Float::toString(15.5f, 16);
  // Usually "0x1.f00000p+3" or similar. Implementation dependent?
  // We check it contains "p" and "0x".
  EXPECT_NE(s.find("0x"), std::string::npos);
  EXPECT_NE(s.find("p"), std::string::npos);
}
