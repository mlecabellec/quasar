#include <gtest/gtest.h>
#include "quasar/coretypes/Variant.hpp"
#include <string>
#include <vector>

using namespace quasar::coretypes;

/**
 * @test VariantTest.BasicConstructionAndTypeChecking
 * @brief Verifies that Variant can be constructed with all supported types and correctly reports its state.
 */
TEST(VariantTest, BasicConstructionAndTypeChecking) {
    // [CS-0010.44] Testing empty variant state.
    {
        Variant v;
        EXPECT_EQ(v.getVariantType(), VariantType::Empty);
        EXPECT_FALSE(v.holds<bool>());
    }

    // [CS-0010.44] Testing boolean variant state.
    {
        Variant v(true);
        EXPECT_EQ(v.getVariantType(), VariantType::Boolean);
        EXPECT_TRUE(v.holds<bool>());
        EXPECT_EQ(v.getAs<bool>(), true);
        EXPECT_EQ(v.toString(), "true");
    }

    // [CS-0010.44] Testing integer variant state.
    {
        Variant v(static_cast<int64_t>(42));
        EXPECT_EQ(v.getVariantType(), VariantType::Integer);
        EXPECT_TRUE(v.holds<int64_t>());
        EXPECT_EQ(v.getAs<int64_t>(), 42);
        EXPECT_EQ(v.toString(), "42");
    }

    // [CS-0010.44] Testing double variant state.
    {
        Variant v(3.14);
        EXPECT_EQ(v.getVariantType(), VariantType::Double);
        EXPECT_TRUE(v.holds<double>());
        EXPECT_DOUBLE_EQ(v.getAs<double>(), 3.14);
        EXPECT_NE(v.toString().find("3.14"), std::string::npos);
    }

    // [CS-0010.44] Testing string variant state.
    {
        std::string testStr = "Quasar";
        Variant v(testStr);
        EXPECT_EQ(v.getVariantType(), VariantType::String);
        EXPECT_TRUE(v.holds<std::string>());
        EXPECT_EQ(v.getAs<std::string>(), "Quasar");
        EXPECT_EQ(v.toString(), "Quasar");
    }

    // [CS-0010.44] Testing buffer variant state.
    {
        std::vector<uint8_t> testBuf = {0xDE, 0xAD, 0xBE, 0xEF};
        Variant v(testBuf);
        EXPECT_EQ(v.getVariantType(), VariantType::Buffer);
        EXPECT_TRUE(v.holds<std::vector<uint8_t>>());
        EXPECT_EQ(v.getAs<std::vector<uint8_t>>().size(), 4);
        EXPECT_EQ(v.toString(), "Buffer[4]");
    }
}

/**
 * @test VariantTest.ErrorHandlingAndSafety
 * @brief Verifies that Variant correctly handles invalid type access and safe pointer retrieval.
 */
TEST(VariantTest, ErrorHandlingAndSafety) {
    // [CS-0010.31] Explicit declaration of the variant.
    Variant v(static_cast<int64_t>(100));

    // [CS-0010.44] Bad Access shall throw std::bad_variant_access.
    EXPECT_THROW(v.getAs<bool>(), std::bad_variant_access);

    // [CS-0010.44] getIf shall return nullptr on type mismatch.
    EXPECT_EQ(v.getIf<bool>(), nullptr);
    EXPECT_NE(v.getIf<int64_t>(), nullptr);
}

/**
 * @test VariantTest.MoveAndCopySemantics
 * @brief Verifies that Variant correctly implements copy and move semantics.
 */
TEST(VariantTest, MoveAndCopySemantics) {
    // [CS-0010.44] Testing copy constructor.
    {
        Variant v1(std::string("CopyMe"));
        Variant v2(v1);
        EXPECT_EQ(v2.getAs<std::string>(), "CopyMe");
        EXPECT_EQ(v1.getAs<std::string>(), "CopyMe");
    }

    // [CS-0010.44] Testing move constructor.
    {
        Variant v1(std::string("MoveMe"));
        Variant v2(std::move(v1));
        EXPECT_EQ(v2.getAs<std::string>(), "MoveMe");
    }
}
