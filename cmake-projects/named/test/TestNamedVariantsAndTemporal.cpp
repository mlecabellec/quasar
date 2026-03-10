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

TEST(NamedVariantsAndTemporalTest, NamedVariant) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedVariant> variant = NamedVariant::create("myVariant");

    // Initially empty
    EXPECT_EQ(variant->get(), nullptr);
    EXPECT_FALSE(variant->holds<NamedInteger<int>>());

    std::shared_ptr<NamedInteger<int>> i = NamedInteger<int>::create("tempVal", 100);
    variant->set(i);

    EXPECT_TRUE(variant->holds<NamedInteger<int>>());
    EXPECT_FALSE(variant->holds<NamedString>());
    
    std::shared_ptr<NamedInteger<int>> retrieved = variant->getAs<NamedInteger<int>>();
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value(), 100);

    // The child should be renamed to "value"
    EXPECT_EQ(retrieved->getName(), "value");

    // Replace
    std::shared_ptr<NamedString> s = NamedString::create("tempStr", "hello");
    variant->set(s);

    EXPECT_FALSE(variant->holds<NamedInteger<int>>());
    EXPECT_TRUE(variant->holds<NamedString>());
    EXPECT_EQ(variant->getAs<NamedString>()->value(), "hello");
}

#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"

TEST(NamedVariantsAndTemporalTest, CollectionVariants) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedArray<NamedVariant>> array = NamedArray<NamedVariant>::create("varArray");
    std::shared_ptr<NamedVariant> v1 = NamedVariant::create("v1");
    std::shared_ptr<NamedInteger<int>> i1 = NamedInteger<int>::create("temp", 42);
    v1->set(i1);
    
    std::shared_ptr<NamedVariant> v2 = NamedVariant::create("v2");
    std::shared_ptr<NamedString> s1 = NamedString::create("temp", "world");
    v2->set(s1);
    
    array->push_back(v1);
    array->push_back(v2);
    
    EXPECT_EQ(array->size(), 2);
    EXPECT_TRUE(array->get(0)->holds<NamedInteger<int>>());
    EXPECT_EQ(array->get(0)->getAs<NamedInteger<int>>()->value(), 42);
    
    EXPECT_TRUE(array->get(1)->holds<NamedString>());
    EXPECT_EQ(array->get(1)->getAs<NamedString>()->value(), "world");
}

TEST(NamedVariantsAndTemporalTest, TemporalTimestampTest) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedTimestamp> ts = NamedTimestamp::create("ts", 1680000000000000); // Some exact microsecond time
    EXPECT_EQ(ts->value(), 1680000000000000);
    EXPECT_EQ(ts->getType(), "NamedTimestamp");

    std::string iso = ts->toISO8601();
    // format YYYY-MM-DDTHH:MM:SS.uuuuuuZ
    std::regex iso_regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{6}Z)");
    EXPECT_TRUE(std::regex_match(iso, iso_regex));

    std::shared_ptr<NamedTimestamp> nowTs = NamedTimestamp::now("nowTs");
    EXPECT_GT(nowTs->value(), 0);
}

TEST(NamedVariantsAndTemporalTest, TemporalDurationTest) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedDuration> dur = NamedDuration::create("dur", 1500000); // 1.5 seconds
    EXPECT_EQ(dur->value(), 1500000);
    EXPECT_DOUBLE_EQ(dur->toSeconds(), 1.5);

    std::shared_ptr<NamedDuration> dur2 = NamedDuration::create("dur2", Duration::fromSeconds(2.5).value());
    EXPECT_DOUBLE_EQ(dur2->toSeconds(), 2.5);
}

TEST(NamedVariantsAndTemporalTest, TemporalDateTest) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedDate> date = NamedDate::create("date", 19000); // some day count
    EXPECT_EQ(date->value(), 19000);
    EXPECT_EQ(date->getType(), "NamedDate");

    std::string iso = date->toISO8601();
    // format YYYY-MM-DD
    std::regex iso_regex(R"(\d{4}-\d{2}-\d{2})");
    EXPECT_TRUE(std::regex_match(iso, iso_regex));
}

