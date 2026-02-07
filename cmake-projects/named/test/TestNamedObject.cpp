#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Serialization.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;

TEST(NamedObjectTest, Creation) {
  // Step: Create NamedObject "root"
  std::cout << "Step: Create NamedObject \"root\"" << std::endl;
  auto obj = NamedObject::create("root");

  // Assertion: Check if name is "root"
  std::cout << "Assertion: Check if name is \"root\"" << std::endl;
  EXPECT_EQ(obj->getName(), "root");

  // Assertion: Check if parent is nullptr
  std::cout << "Assertion: Check if parent is nullptr" << std::endl;
  EXPECT_EQ(obj->getParent(), nullptr);
}

TEST(NamedObjectTest, InvalidName) {
  // Assertion: Check if empty name throws
  std::cout << "Assertion: Check if empty name throws" << std::endl;
  EXPECT_THROW(NamedObject::create(""), std::runtime_error);

  // Assertion: Check if numeric start throws
  std::cout << "Assertion: Check if numeric start throws" << std::endl;
  EXPECT_THROW(NamedObject::create("123"),
               std::runtime_error); // Must start with letter/_

  // Assertion: Check if invalid char throws
  std::cout << "Assertion: Check if invalid char throws" << std::endl;
  EXPECT_THROW(NamedObject::create("invalid-name"), std::runtime_error);
}

TEST(NamedObjectTest, Hierarchy) {
  // Step: Create root and child1
  std::cout << "Step: Create root and child1" << std::endl;
  auto root = NamedObject::create("root");
  auto child1 = NamedObject::create("child1", root);

  // Assertion: Check child1 parent is root
  std::cout << "Assertion: Check child1 parent is root" << std::endl;
  EXPECT_EQ(child1->getParent(), root);

  // Assertion: Check root children size is 1
  std::cout << "Assertion: Check root children size is 1" << std::endl;
  EXPECT_EQ(root->getChildren().size(), 1);

  // Assertion: Check first child is child1
  std::cout << "Assertion: Check first child is child1" << std::endl;
  EXPECT_EQ(root->getFirstChild(), child1);

  // Step: Create child2 and set parent to root
  std::cout << "Step: Create child2 and set parent to root" << std::endl;
  auto child2 = NamedObject::create("child2");
  child2->setParent(root);

  // Assertion: Check root children size is 2
  std::cout << "Assertion: Check root children size is 2" << std::endl;
  EXPECT_EQ(root->getChildren().size(), 2);

  // Assertion: Check sibling links
  std::cout << "Assertion: Check if child2 is next sibling of child1"
            << std::endl;
  EXPECT_EQ(child1->getNextSibling(), child2);
  std::cout << "Assertion: Check if child1 is previous sibling of child2"
            << std::endl;
  EXPECT_EQ(child2->getPreviousSibling(), child1);

  // Step: Remove child2 parent
  std::cout << "Step: Remove child2 parent" << std::endl;
  child2->setParent(nullptr);

  // Assertion: Check root children size is 1
  std::cout << "Assertion: Check root children size is 1" << std::endl;
  EXPECT_EQ(root->getChildren().size(), 1);

  // Assertion: Check child2 parent is nullptr
  std::cout << "Assertion: Check child2 parent is nullptr" << std::endl;
  EXPECT_EQ(child2->getParent(), nullptr);
}

TEST(NamedObjectTest, Uniqueness) {
  // Step: Create root and child with name "child"
  std::cout << "Step: Create root and child with name \"child\"" << std::endl;
  auto root = NamedObject::create("root");
  auto child1 = NamedObject::create("child", root);

  // Assertion: Check if creating another child with same name throws
  std::cout
      << "Assertion: Check if creating another child with same name throws"
      << std::endl;
  EXPECT_THROW(NamedObject::create("child", root), std::runtime_error);

  // Step: Create standalone child2 with name "child"
  std::cout << "Step: Create standalone child2 with name \"child\""
            << std::endl;
  auto child2 = NamedObject::create("child");

  // Assertion: Check if setting parent to root throws due to name collision
  std::cout << "Assertion: Check if setting parent to root throws due to name "
               "collision"
            << std::endl;
  EXPECT_THROW(child2->setParent(root), std::runtime_error);
}

TEST(NamedObjectTest, CycleDetection) {
  // Step: Create p1 -> p2 -> p3 hierarchy
  std::cout << "Step: Create p1 -> p2 -> p3 hierarchy" << std::endl;
  auto p1 = NamedObject::create("p1");
  auto p2 = NamedObject::create("p2", p1);
  auto p3 = NamedObject::create("p3", p2);

  // Assertion: Check if setting p1 parent to p3 throws (cycle)
  std::cout << "Assertion: Check if setting p1 parent to p3 throws (cycle)"
            << std::endl;
  EXPECT_THROW(p1->setParent(p3), std::runtime_error);

  // Assertion: Check if setting p1 parent to itself throws
  std::cout << "Assertion: Check if setting p1 parent to itself throws"
            << std::endl;
  EXPECT_THROW(p1->setParent(p1), std::runtime_error);
}

TEST(NamedObjectTest, DerivedClasses) {
  // Step: Create root, NamedInteger and NamedBoolean
  std::cout << "Step: Create root, NamedInteger and NamedBoolean" << std::endl;
  auto root = NamedObject::create("root");
  auto i = NamedInteger<int>::create("intVal", 42, root);
  auto b = NamedBoolean::create("boolVal", true, root);

  // Assertion: Check NamedInteger value
  std::cout << "Assertion: Check NamedInteger value is 42" << std::endl;
  EXPECT_EQ(i->toInt(), 42);

  // Assertion: Check NamedBoolean value
  std::cout << "Assertion: Check NamedBoolean value is true" << std::endl;
  EXPECT_TRUE(b->booleanValue());

  // Assertion: Check root children size is 2
  std::cout << "Assertion: Check root children size is 2" << std::endl;
  EXPECT_EQ(root->getChildren().size(), 2);
}

TEST(NamedObjectTest, SerializationXML) {
  // Step: Create objects for XML serialization
  std::cout << "Step: Create objects for XML serialization" << std::endl;
  auto root = NamedObject::create("root");
  auto child = NamedInteger<int>::create("val", 123, root);

  // Step: Serialize to XML
  std::cout << "Step: Serialize to XML" << std::endl;
  std::string xml = serialization::toXml(root);

  // Assertion: Check XML is not empty
  std::cout << "Assertion: Check XML is not empty" << std::endl;
  EXPECT_FALSE(xml.empty());

  // Assertion: Check if "root" and "val" are in XML
  std::cout << "Assertion: Check if \"root\" is in XML" << std::endl;
  EXPECT_NE(xml.find("name=\"root\""), std::string::npos);
  std::cout << "Assertion: Check if \"val\" is in XML" << std::endl;
  EXPECT_NE(xml.find("name=\"val\""), std::string::npos);
}

TEST(NamedObjectTest, SerializationYAML) {
  // Step: Create objects for YAML serialization
  std::cout << "Step: Create objects for YAML serialization" << std::endl;
  auto root = NamedObject::create("root");
  auto child = NamedInteger<int>::create("val", 123, root);

  // Step: Serialize to YAML
  std::cout << "Step: Serialize to YAML" << std::endl;
  std::string yaml = serialization::toYaml(root);

  // Assertion: Check YAML is not empty
  std::cout << "Assertion: Check YAML is not empty" << std::endl;
  EXPECT_FALSE(yaml.empty());

  // Assertion: Check if "root" is in YAML
  std::cout << "Assertion: Check if \"root\" is in YAML" << std::endl;
  EXPECT_NE(yaml.find("name: root"), std::string::npos);
}

TEST(NamedObjectTest, NamedBuffers) {
  // Step: Create NamedBuffer
  std::cout << "Step: Create NamedBuffer" << std::endl;
  auto root = NamedObject::create("root");
  std::vector<uint8_t> data = {0x01, 0x02};
  auto nb = NamedBuffer::create("buffer", data, root);

  // Assertion: Check NamedBuffer size and value
  std::cout << "Assertion: Check NamedBuffer size is 2" << std::endl;
  EXPECT_EQ(nb->size(), 2);
  std::cout << "Assertion: Check NamedBuffer index 0 is 0x01" << std::endl;
  EXPECT_EQ(nb->get(0), 0x01);

  // Step: Create NamedBitBuffer
  std::cout << "Step: Create NamedBitBuffer" << std::endl;
  auto nbb = NamedBitBuffer::create("bitbuffer", 16, root);
  nbb->setBit(0, true);

  // Assertion: Check NamedBitBuffer size and value
  std::cout << "Assertion: Check NamedBitBuffer bitSize is 16" << std::endl;
  EXPECT_EQ(nbb->bitSize(), 16);
  std::cout << "Assertion: Check NamedBitBuffer bit 0 is true" << std::endl;
  EXPECT_TRUE(nbb->getBit(0));
}

TEST(NamedObjectTest, RelatedObject) {
  // Step: Create two objects
  std::cout << "Step: Create two objects" << std::endl;
  auto obj1 = NamedObject::create("obj1");
  auto obj2 = NamedObject::create("obj2");

  // Assertion: Initial related object should be null
  std::cout << "Assertion: Initial related object should be nullptr"
            << std::endl;
  EXPECT_EQ(obj1->getRelated(), nullptr);

  // Step: Link obj1 to obj2
  std::cout << "Step: Link obj1 to obj2" << std::endl;
  obj1->setRelated(obj2);

  // Assertion: Related object should be obj2
  std::cout << "Assertion: Related object should be obj2" << std::endl;
  EXPECT_EQ(obj1->getRelated(), obj2);

  // Step: Reset obj2 (destroy it)
  std::cout << "Step: Reset obj2 (destroy it)" << std::endl;
  obj2.reset();

  // Assertion: Related object should become null (weak reference)
  std::cout << "Assertion: Related object should be nullptr" << std::endl;
  EXPECT_EQ(obj1->getRelated(), nullptr);
}
