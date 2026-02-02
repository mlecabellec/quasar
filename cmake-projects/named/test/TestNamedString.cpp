#include <gtest/gtest.h>
#include "quasar/named/NamedString.hpp"

using namespace quasar::named;

TEST(NamedStringTest, CreationAndValue) {
    auto ns = NamedString::create("myString", "Hello World");
    EXPECT_EQ(ns->getName(), "myString");
    EXPECT_EQ(ns->toString(), "Hello World");
    EXPECT_EQ(ns->length(), 11);
}

TEST(NamedStringTest, ParentChild) {
    auto root = NamedObject::create("root");
    auto ns = NamedString::create("childString", "Value", root);
    
    EXPECT_EQ(ns->getParent(), root);
    EXPECT_EQ(root->getChildren().size(), 1);
}

TEST(NamedStringTest, Clone) {
    auto ns = NamedString::create("original", "content");
    auto copy = ns->clone();
    
    EXPECT_EQ(copy->getName(), "original");
    auto casted = std::dynamic_pointer_cast<NamedString>(copy);
    ASSERT_NE(casted, nullptr);
    EXPECT_EQ(casted->toString(), "content");
}
