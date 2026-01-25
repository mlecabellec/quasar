#include <gtest/gtest.h>
#include <quasar/coretypes/coretypes.hpp>

using namespace quasar::coretypes;

TEST(IntegerTest, Construction)
{
    Integer a(5);
    EXPECT_EQ(a.toString(), "5");
}

TEST(IntegerTest, Arithmetic)
{
    Integer a(3), b(4);
    EXPECT_EQ((a + b).toString(), "7");
}
