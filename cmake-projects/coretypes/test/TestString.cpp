#include "quasar/coretypes/String.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::coretypes;

TEST(StringTest, Creation) {
  // Step: Initialize empty String
  std::cout << "Step: Initialize empty String" << std::endl;
  String s1;

  // Assertion: Check if string is empty
  std::cout << "Assertion: Check if string is empty" << std::endl;
  EXPECT_TRUE(s1.empty());

  // Assertion: Check if length is 0
  std::cout << "Assertion: Check if length is 0" << std::endl;
  EXPECT_EQ(s1.length(), 0);

  // Step: Initialize String with "hello"
  std::cout << "Step: Initialize String with \"hello\"" << std::endl;
  String s2("hello");

  // Assertion: Check if value is "hello"
  std::cout << "Assertion: Check if value is \"hello\"" << std::endl;
  EXPECT_EQ(s2.toString(), "hello");

  // Assertion: Check if length is 5
  std::cout << "Assertion: Check if length is 5" << std::endl;
  EXPECT_EQ(s2.length(), 5);

  // Step: Initialize String with std::string("world")
  std::cout << "Step: Initialize String with std::string(\"world\")"
            << std::endl;
  String s3(std::string("world"));

  // Assertion: Check if value is "world"
  std::cout << "Assertion: Check if value is \"world\"" << std::endl;
  EXPECT_EQ(s3.toString(), "world");
}

TEST(StringTest, Comparison) {
  // Step: Initialize Strings 's1', 's2', 's3'
  std::cout << "Step: Initialize Strings 's1', 's2', 's3'" << std::endl;
  String s1("abc");
  String s2("abc");
  String s3("def");

  // Assertion: Check if s1 equals s2
  std::cout << "Assertion: Check if s1 equals s2" << std::endl;
  EXPECT_TRUE(s1.equals(s2));

  // Assertion: Check if s1 does not equal s3
  std::cout << "Assertion: Check if s1 does not equal s3" << std::endl;
  EXPECT_FALSE(s1.equals(s3));

  // Assertion: Check if compareTo for equal strings is 0
  std::cout << "Assertion: Check if compareTo for equal strings is 0"
            << std::endl;
  EXPECT_EQ(s1.compareTo(s2), 0);

  // Assertion: Check if s1 < s3
  std::cout << "Assertion: Check if s1 < s3" << std::endl;
  EXPECT_LT(s1.compareTo(s3), 0);

  // Assertion: Check if s3 > s1
  std::cout << "Assertion: Check if s3 > s1" << std::endl;
  EXPECT_GT(s3.compareTo(s1), 0);
}
