#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include <gtest/gtest.h>
#include <regex>

using namespace quasar::named;
using namespace quasar::coretypes;

/**
 * @test NamedVariantsAndTemporalTest.NamedVariant
 * @brief Verifies the basic functionality of the new value-based NamedVariant.
 */
TEST(NamedVariantsAndTemporalTest, NamedVariant) {
    // [CS-0010.31] Explicit type declaration.
    std::shared_ptr<NamedVariant> variant = NamedVariant::create("myVariant");

    // [CS-0010.44] Initially empty state.
    EXPECT_EQ(variant->getVariantType(), VariantType::Empty);
    EXPECT_FALSE(variant->getVariant().holds<int64_t>());

    // [CS-0010.44] Setting an integer value.
    variant->setVariant(Variant(static_cast<int64_t>(100)));

    EXPECT_TRUE(variant->getVariant().holds<int64_t>());
    EXPECT_FALSE(variant->getVariant().holds<std::string>());
    
    EXPECT_EQ(variant->getVariant().getAs<int64_t>(), 100);

    // [CS-0010.44] Replacing with a string value.
    variant->setVariant(Variant(std::string("hello")));

    EXPECT_FALSE(variant->getVariant().holds<int64_t>());
    EXPECT_TRUE(variant->getVariant().holds<std::string>());
    EXPECT_EQ(variant->getVariant().getAs<std::string>(), "hello");
}

#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"

/**
 * @test NamedVariantsAndTemporalTest.CollectionVariants
 * @brief Verifies that NamedVariants can be stored in named collections.
 */
TEST(NamedVariantsAndTemporalTest, CollectionVariants) {
    // [CS-0010.31] Explicit type declarations.
    std::shared_ptr<NamedArray<NamedVariant>> array = NamedArray<NamedVariant>::create("varArray");
    std::shared_ptr<NamedVariant> v1 = NamedVariant::create("v1", Variant(static_cast<int64_t>(42)));
    std::shared_ptr<NamedVariant> v2 = NamedVariant::create("v2", Variant(std::string("world")));
    
    array->push_back(v1);
    array->push_back(v2);
    
    // [CS-0010.44] Verifying collection content.
    EXPECT_EQ(array->size(), 2);
    EXPECT_TRUE(array->get(0)->getVariant().holds<int64_t>());
    EXPECT_EQ(array->get(0)->getVariant().getAs<int64_t>(), 42);
    
    EXPECT_TRUE(array->get(1)->getVariant().holds<std::string>());
    EXPECT_EQ(array->get(1)->getVariant().getAs<std::string>(), "world");
}

/**
 * @test NamedVariantsAndTemporalTest.TemporalTimestampTest
 * @brief Verifies the functionality of the NamedTimestamp class.
 */
TEST(NamedVariantsAndTemporalTest, TemporalTimestampTest) {
    // [CS-0010.31] Explicit type declaration.
    std::shared_ptr<NamedTimestamp> ts = NamedTimestamp::create("ts", 1680000000000000); 
    EXPECT_EQ(ts->value(), 1680000000000000);
    EXPECT_EQ(ts->getType(), "NamedTimestamp");

    // [CS-0010.44] Checking ISO 8601 formatting.
    std::string iso = ts->toISO8601();
    static const std::regex iso_regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{6}Z)");
    EXPECT_TRUE(std::regex_match(iso, iso_regex));

    std::shared_ptr<NamedTimestamp> nowTs = NamedTimestamp::now("nowTs");
    EXPECT_GT(nowTs->value(), 0);
}

/**
 * @test NamedVariantsAndTemporalTest.TemporalDurationTest
 * @brief Verifies the functionality of the NamedDuration class.
 */
TEST(NamedVariantsAndTemporalTest, TemporalDurationTest) {
    // [CS-0010.31] Explicit type declaration.
    std::shared_ptr<NamedDuration> dur = NamedDuration::create("dur", 1500000); // 1.5 seconds
    EXPECT_EQ(dur->value(), 1500000);
    EXPECT_DOUBLE_EQ(dur->toSeconds(), 1.5);

    std::shared_ptr<NamedDuration> dur2 = NamedDuration::create("dur2", Duration::fromSeconds(2.5).value());
    EXPECT_DOUBLE_EQ(dur2->toSeconds(), 2.5);
}

/**
 * @test NamedVariantsAndTemporalTest.TemporalDateTest
 * @brief Verifies the functionality of the NamedDate class.
 */
TEST(NamedVariantsAndTemporalTest, TemporalDateTest) {
    // [CS-0010.31] Explicit type declaration.
    std::shared_ptr<NamedDate> date = NamedDate::create("date", 19000); 
    EXPECT_EQ(date->value(), 19000);
    EXPECT_EQ(date->getType(), "NamedDate");

    // [CS-0010.44] Checking ISO 8601 formatting.
    std::string iso = date->toISO8601();
    static const std::regex iso_regex(R"(\d{4}-\d{2}-\d{2})");
    EXPECT_TRUE(std::regex_match(iso, iso_regex));
}
