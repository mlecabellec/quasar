#include <gtest/gtest.h>
#include "quasar/named/NamedVariant.hpp"
#include <string>
#include <vector>

using namespace quasar::named;
using namespace quasar::coretypes;

/**
 * @test NamedVariantTest.BasicUsage
 * @brief Verifies that NamedVariant correctly holds and allows updating its internal variant value.
 */
TEST(NamedVariantTest, BasicUsage) {
    // [CS-0010.31] Explicit type declaration instead of auto.
    std::shared_ptr<NamedVariant> v = NamedVariant::create("myVar", Variant(int64_t(123)));
    
    // [CS-0010.44] Checking initial state.
    EXPECT_EQ(v->getName(), "myVar");
    EXPECT_EQ(v->getType(), "NamedVariant");
    EXPECT_EQ(v->getVariantType(), VariantType::Integer);
    EXPECT_EQ(v->getVariant().getAs<int64_t>(), 123);
    
    // [CS-0010.44] Updating the value.
    v->setVariant(Variant(std::string("hello")));
    EXPECT_EQ(v->getVariant().getAs<std::string>(), "hello");
}

/**
 * @test NamedVariantTest.Cloning
 * @brief Verifies that NamedVariant correctly implements the clone operation.
 */
TEST(NamedVariantTest, Cloning) {
    // [CS-0010.31] Explicit type declaration instead of auto.
    std::shared_ptr<NamedVariant> v1 = NamedVariant::create("v1", Variant(3.14));
    
    // [CS-0010.44] Cloning and casting back.
    std::shared_ptr<NamedVariant> v2 = std::dynamic_pointer_cast<NamedVariant>(v1->clone());
    
    ASSERT_NE(v2, nullptr);
    EXPECT_EQ(v2->getName(), "v1");
    EXPECT_DOUBLE_EQ(v2->getVariant().getAs<double>(), 3.14);
}
