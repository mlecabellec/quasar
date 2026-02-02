#include "quasar/coretypes/Boolean.hpp"
#include <gtest/gtest.h>

using namespace quasar::coretypes;

TEST(BooleanTest, ConstructorValue) {
  Boolean b(true);
  EXPECT_TRUE(b.booleanValue());
  Boolean b2(false);
  EXPECT_FALSE(b2.booleanValue());
}

TEST(BooleanTest, ConstructorString) {
  Boolean b1("true");
  EXPECT_TRUE(b1.booleanValue());
  Boolean b2("TRUE");
  EXPECT_TRUE(b2.booleanValue());
  Boolean b3("false");
  EXPECT_FALSE(b3.booleanValue());
  Boolean b4("random");
  EXPECT_FALSE(b4.booleanValue());
}

TEST(BooleanTest, ToString) {
  Boolean b(true);
  EXPECT_EQ(b.toString(), "true");
  Boolean b2(false);
  EXPECT_EQ(b2.toString(), "false");
}

TEST(BooleanTest, FromNumeric) {
  Boolean b1 = Boolean::fromNumeric(1);
  EXPECT_TRUE(b1.booleanValue());
  Boolean b2 = Boolean::fromNumeric(0);
  EXPECT_FALSE(b2.booleanValue());
  Boolean b3 = Boolean::fromNumeric(3.14);
  EXPECT_TRUE(b3.booleanValue());
}

TEST(BooleanTest, PrimitiveComparison) {
  Boolean b(true);
  EXPECT_TRUE(b.equals(true));
  EXPECT_FALSE(b.equals(false));
  
  EXPECT_EQ(b.compareTo(true), 0);
  EXPECT_GT(b.compareTo(false), 0);
  
  EXPECT_TRUE(b == true);
  EXPECT_TRUE(b != false);
}
