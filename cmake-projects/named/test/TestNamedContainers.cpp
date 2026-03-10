#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/NamedSet.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <gtest/gtest.h>

using namespace quasar::named;

TEST(NamedContainersTest, NamedArray) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedArray<NamedInteger<int>>> array = NamedArray<NamedInteger<int>>::create("myArray");
    EXPECT_EQ(array->size(), 0);

    std::shared_ptr<NamedInteger<int>> item1 = NamedInteger<int>::create("temp1", 42);
    std::shared_ptr<NamedInteger<int>> item2 = NamedInteger<int>::create("temp2", 84);

    array->push_back(item1);
    array->push_back(item2);

    EXPECT_EQ(array->size(), 2);
    // Elements should be renamed to their index
    EXPECT_EQ(array->get(0)->getName(), "_0");
    EXPECT_EQ(array->get(1)->getName(), "_1");
    // Elements should be accessible as children
    EXPECT_EQ(array->getChildren().size(), 2);

    // Remove element at index 0
    array->removeAt(0);

    // Size should be 1, the former item2 should now be at index 0 and named "_0"
    EXPECT_EQ(array->size(), 1);
    EXPECT_EQ(array->get(0)->value(), 84);
    EXPECT_EQ(array->get(0)->getName(), "_0");
}

TEST(NamedContainersTest, NamedMap) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedMap<NamedInteger<int>>> map = NamedMap<NamedInteger<int>>::create("myMap");
    EXPECT_EQ(map->size(), 0);

    std::shared_ptr<NamedInteger<int>> item = NamedInteger<int>::create("temp", 100);
    map->put("key1", item);

    EXPECT_EQ(map->size(), 1);
    EXPECT_EQ(item->getName(), "key1");
    EXPECT_TRUE(map->contains("key1"));
    EXPECT_EQ(map->get("key1")->value(), 100);

    // Replace
    std::shared_ptr<NamedInteger<int>> item2 = NamedInteger<int>::create("temp2", 200);
    map->put("key1", item2);

    EXPECT_EQ(map->size(), 1);
    EXPECT_EQ(map->get("key1")->value(), 200);

    // Remove
    EXPECT_TRUE(map->remove("key1"));
    EXPECT_EQ(map->size(), 0);
    EXPECT_FALSE(map->remove("key1")); // Safe to remove non-existent
}

TEST(NamedContainersTest, NamedSet) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedSet<NamedInteger<int>>> set = NamedSet<NamedInteger<int>>::create("mySet");
    EXPECT_EQ(set->size(), 0);

    std::shared_ptr<NamedInteger<int>> item1 = NamedInteger<int>::create("keyA", 1);
    std::shared_ptr<NamedInteger<int>> item2 = NamedInteger<int>::create("keyB", 2);

    set->insert(item1);
    set->insert(item2);

    EXPECT_EQ(set->size(), 2);
    EXPECT_TRUE(set->contains("keyA"));
    EXPECT_TRUE(set->contains("keyB"));
    EXPECT_EQ(set->get("keyA")->value(), 1);

    // Removing
    EXPECT_TRUE(set->remove("keyA"));
    EXPECT_EQ(set->size(), 1);
    EXPECT_FALSE(set->contains("keyA"));

    // Replacing
    std::shared_ptr<NamedInteger<int>> item2_updated = NamedInteger<int>::create("keyB", 200);
    set->insert(item2_updated);
    EXPECT_EQ(set->size(), 1);
    EXPECT_EQ(set->get("keyB")->value(), 200);
}

TEST(NamedContainersTest, Iterators) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedArray<NamedInteger<int>>> array = NamedArray<NamedInteger<int>>::create("myArray");
    array->push_back(NamedInteger<int>::create("item1", 10));
    array->push_back(NamedInteger<int>::create("item2", 20));
    array->push_back(NamedInteger<int>::create("item3", 30));

    int sum = 0;
    for (std::vector<std::shared_ptr<NamedInteger<int>>>::iterator it = array->begin(); it != array->end(); ++it) {
        sum += (*it)->value();
    }
    EXPECT_EQ(sum, 60);

    std::shared_ptr<NamedMap<NamedInteger<int>>> map = NamedMap<NamedInteger<int>>::create("myMap");
    map->put("A", NamedInteger<int>::create("A", 100));
    map->put("B", NamedInteger<int>::create("B", 200));

    int mapSum = 0;
    for (std::map<std::string, std::shared_ptr<NamedInteger<int>>>::iterator it = map->begin(); it != map->end(); ++it) {
        mapSum += it->second->value();
    }
    EXPECT_EQ(mapSum, 300);
}

