#include "quasar/coretypes/String.hpp"
#include <gtest/gtest.h>

using namespace quasar::coretypes;

TEST(StringTest, Creation) {
  String s1;
  EXPECT_TRUE(s1.empty());
  EXPECT_EQ(s1.length(), 0);

  String s2("hello");
  EXPECT_EQ(s2.toString(), "hello");
  EXPECT_EQ(s2.length(), 5);

  String s3(std::string("world"));
  EXPECT_EQ(s3.toString(), "world");
}

TEST(StringTest, Comparison) {
  String s1("abc");
  String s2("abc");
  String s3("def");

  EXPECT_TRUE(s1.equals(s2));
  EXPECT_FALSE(s1.equals(s3));

  EXPECT_EQ(s1.compareTo(s2), 0);
  EXPECT_LT(s1.compareTo(s3), 0);
  EXPECT_GT(s3.compareTo(s1), 0);
}
