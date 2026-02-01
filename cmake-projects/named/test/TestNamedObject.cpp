#include <gtest/gtest.h>
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/Serialization.hpp"

using namespace quasar::named;

TEST(NamedObjectTest, Creation) {
    auto obj = NamedObject::create("root");
    EXPECT_EQ(obj->getName(), "root");
    EXPECT_EQ(obj->getParent(), nullptr);
}

TEST(NamedObjectTest, InvalidName) {
    EXPECT_THROW(NamedObject::create(""), std::runtime_error);
    EXPECT_THROW(NamedObject::create("123"), std::runtime_error); // Must start with letter/_ 
    EXPECT_THROW(NamedObject::create("invalid-name"), std::runtime_error);
}

TEST(NamedObjectTest, Hierarchy) {
    auto root = NamedObject::create("root");
    auto child1 = NamedObject::create("child1", root);
    
    EXPECT_EQ(child1->getParent(), root);
    EXPECT_EQ(root->getChildren().size(), 1);
    EXPECT_EQ(root->getFirstChild(), child1);
    
    auto child2 = NamedObject::create("child2");
    child2->setParent(root);
    
    EXPECT_EQ(root->getChildren().size(), 2);
    EXPECT_EQ(child1->getNextSibling(), child2);
    EXPECT_EQ(child2->getPreviousSibling(), child1);
    
    child2->setParent(nullptr);
    EXPECT_EQ(root->getChildren().size(), 1);
    EXPECT_EQ(child2->getParent(), nullptr);
}

TEST(NamedObjectTest, Uniqueness) {
    auto root = NamedObject::create("root");
    auto child1 = NamedObject::create("child", root);
    
    EXPECT_THROW(NamedObject::create("child", root), std::runtime_error);
    
    auto child2 = NamedObject::create("child");
    EXPECT_THROW(child2->setParent(root), std::runtime_error);
}

TEST(NamedObjectTest, CycleDetection) {
    auto p1 = NamedObject::create("p1");
    auto p2 = NamedObject::create("p2", p1);
    auto p3 = NamedObject::create("p3", p2);
    
    EXPECT_THROW(p1->setParent(p3), std::runtime_error);
    EXPECT_THROW(p1->setParent(p1), std::runtime_error);
}

TEST(NamedObjectTest, DerivedClasses) {
    auto root = NamedObject::create("root");
    auto i = NamedInteger<int>::create("intVal", 42, root);
    auto b = NamedBoolean::create("boolVal", true, root);
    
    EXPECT_EQ(i->toInt(), 42);
    EXPECT_TRUE(b->booleanValue());
    EXPECT_EQ(root->getChildren().size(), 2);
}

TEST(NamedObjectTest, SerializationXML) {
    auto root = NamedObject::create("root");
    auto child = NamedInteger<int>::create("val", 123, root);
    
    std::string xml = serialization::toXml(root);
    EXPECT_FALSE(xml.empty());
    // Basic checks
    EXPECT_NE(xml.find("name=\"root\""), std::string::npos);
    EXPECT_NE(xml.find("name=\"val\""), std::string::npos);
}

TEST(NamedObjectTest, SerializationYAML) {
    auto root = NamedObject::create("root");
    auto child = NamedInteger<int>::create("val", 123, root);
    
    std::string yaml = serialization::toYaml(root);
    EXPECT_FALSE(yaml.empty());
    EXPECT_NE(yaml.find("name: root"), std::string::npos);
}
