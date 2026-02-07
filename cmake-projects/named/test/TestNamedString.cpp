#include "quasar/named/NamedString.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;

TEST(NamedStringTest, CreationAndValue) {
  // Step: Create NamedString with name "myString" and value "Hello World"
  std::cout
      << "Step: Create NamedString \"myString\" with value \"Hello World\""
      << std::endl;
  auto ns = NamedString::create("myString", "Hello World");

  // Assertion: Check if name matches
  std::cout << "Assertion: Check if name is \"myString\"" << std::endl;
  EXPECT_EQ(ns->getName(), "myString");

  // Assertion: Check if value matches
  std::cout << "Assertion: Check if value is \"Hello World\"" << std::endl;
  EXPECT_EQ(ns->toString(), "Hello World");

  // Assertion: Check if length matches
  std::cout << "Assertion: Check if length is 11" << std::endl;
  EXPECT_EQ(ns->length(), 11);
}

TEST(NamedStringTest, ParentChild) {
  // Step: Create root NamedObject and NamedString child
  std::cout << "Step: Create root NamedObject and NamedString child"
            << std::endl;
  auto root = NamedObject::create("root");
  auto ns = NamedString::create("childString", "Value", root);

  // Assertion: Check if parent is root
  std::cout << "Assertion: Check if parent is root" << std::endl;
  EXPECT_EQ(ns->getParent(), root);

  // Assertion: Check root children size
  std::cout << "Assertion: Check root children size is 1" << std::endl;
  EXPECT_EQ(root->getChildren().size(), 1);
}

TEST(NamedStringTest, Clone) {
  // Step: Create NamedString and clone it
  std::cout << "Step: Create NamedString and clone it" << std::endl;
  auto ns = NamedString::create("original", "content");
  auto copy = ns->clone();

  // Assertion: Check if clone name matches
  std::cout << "Assertion: Check if clone name is \"original\"" << std::endl;
  EXPECT_EQ(copy->getName(), "original");

  // Step: Cast clone to NamedString
  std::cout << "Step: Cast clone to NamedString" << std::endl;
  auto casted = std::dynamic_pointer_cast<NamedString>(copy);

  // Assertion: Check if cast was successful
  std::cout << "Assertion: Check if cast was successful" << std::endl;
  ASSERT_NE(casted, nullptr);

  // Assertion: Check if clone value matches
  std::cout << "Assertion: Check if clone value is \"content\"" << std::endl;
  EXPECT_EQ(casted->toString(), "content");
}
