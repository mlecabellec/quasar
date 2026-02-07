#include "quasar/coretypes/Boolean.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::coretypes;

TEST(BooleanTest, ConstructorValue) {
  // Step: Initialize Boolean with true
  std::cout << "Step: Initialize Boolean with true" << std::endl;
  Boolean b(true);

  // Assertion: Check if value is true
  std::cout << "Assertion: Check if value is true" << std::endl;
  EXPECT_TRUE(b.booleanValue());

  // Step: Initialize Boolean with false
  std::cout << "Step: Initialize Boolean with false" << std::endl;
  Boolean b2(false);

  // Assertion: Check if value is false
  std::cout << "Assertion: Check if value is false" << std::endl;
  EXPECT_FALSE(b2.booleanValue());
}

TEST(BooleanTest, ConstructorString) {
  // Step: Initialize Boolean from "true" string
  std::cout << "Step: Initialize Boolean from \"true\" string" << std::endl;
  Boolean b1("true");

  // Assertion: Check if value is true
  std::cout << "Assertion: Check if value is true" << std::endl;
  EXPECT_TRUE(b1.booleanValue());

  // Step: Initialize Boolean from \"TRUE\" string
  std::cout << "Step: Initialize Boolean from \"TRUE\" string" << std::endl;
  Boolean b2("TRUE");

  // Assertion: Check if value is true
  std::cout << "Assertion: Check if value is true" << std::endl;
  EXPECT_TRUE(b2.booleanValue());

  // Step: Initialize Boolean from \"false\" string
  std::cout << "Step: Initialize Boolean from \"false\" string" << std::endl;
  Boolean b3("false");

  // Assertion: Check if value is false
  std::cout << "Assertion: Check if value is false" << std::endl;
  EXPECT_FALSE(b3.booleanValue());

  // Step: Initialize Boolean from random string
  std::cout << "Step: Initialize Boolean from random string" << std::endl;
  Boolean b4("random");

  // Assertion: Check if value is false
  std::cout << "Assertion: Check if value is false" << std::endl;
  EXPECT_FALSE(b4.booleanValue());
}

TEST(BooleanTest, ToString) {
  // Step: Initialize Boolean with true and convert to string
  std::cout << "Step: Initialize Boolean with true and convert to string"
            << std::endl;
  Boolean b(true);

  // Assertion: Check if string is "true"
  std::cout << "Assertion: Check if string is \"true\"" << std::endl;
  EXPECT_EQ(b.toString(), "true");

  // Step: Initialize Boolean with false and convert to string
  std::cout << "Step: Initialize Boolean with false and convert to string"
            << std::endl;
  Boolean b2(false);

  // Assertion: Check if string is \"false\"
  std::cout << "Assertion: Check if string is \"false\"" << std::endl;
  EXPECT_EQ(b2.toString(), "false");
}

TEST(BooleanTest, FromNumeric) {
  // Step: Create Boolean from numeric 1
  std::cout << "Step: Create Boolean from numeric 1" << std::endl;
  Boolean b1 = Boolean::fromNumeric(1);

  // Assertion: Check if value is true
  std::cout << "Assertion: Check if value is true" << std::endl;
  EXPECT_TRUE(b1.booleanValue());

  // Step: Create Boolean from numeric 0
  std::cout << "Step: Create Boolean from numeric 0" << std::endl;
  Boolean b2 = Boolean::fromNumeric(0);

  // Assertion: Check if value is false
  std::cout << "Assertion: Check if value is false" << std::endl;
  EXPECT_FALSE(b2.booleanValue());

  // Step: Create Boolean from numeric 3.14
  std::cout << "Step: Create Boolean from numeric 3.14" << std::endl;
  Boolean b3 = Boolean::fromNumeric(3.14);

  // Assertion: Check if value is true
  std::cout << "Assertion: Check if value is true" << std::endl;
  EXPECT_TRUE(b3.booleanValue());
}

TEST(BooleanTest, PrimitiveComparison) {
  // Step: Initialize Boolean with true
  std::cout << "Step: Initialize Boolean with true" << std::endl;
  Boolean b(true);

  // Assertion: Check if equals(true) is true
  std::cout << "Assertion: Check if equals(true) is true" << std::endl;
  EXPECT_TRUE(b.equals(true));

  // Assertion: Check if equals(false) is false
  std::cout << "Assertion: Check if equals(false) is false" << std::endl;
  EXPECT_FALSE(b.equals(false));

  // Assertion: Check if compareTo(true) is 0
  std::cout << "Assertion: Check if compareTo(true) is 0" << std::endl;
  EXPECT_EQ(b.compareTo(true), 0);

  // Assertion: Check if compareTo(false) is greater than 0
  std::cout << "Assertion: Check if compareTo(false) is greater than 0"
            << std::endl;
  EXPECT_GT(b.compareTo(false), 0);

  // Assertion: Check operator== with true
  std::cout << "Assertion: Check operator== with true" << std::endl;
  EXPECT_TRUE(b == true);

  // Assertion: Check operator!= with false
  std::cout << "Assertion: Check operator!= with false" << std::endl;
  EXPECT_TRUE(b != false);
}
