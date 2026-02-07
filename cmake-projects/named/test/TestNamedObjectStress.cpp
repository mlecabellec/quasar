#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Traversal.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace quasar::named;

TEST(NamedObjectStress, DeepHierarchy) {
  // Step: Create deep hierarchy (1000 levels)
  std::cout << "Step: Create deep hierarchy (1000 levels)" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedObject> current = root;

  int depth = 1000;
  for (int i = 0; i < depth; ++i) {
    auto next = NamedObject::create("node_" + std::to_string(i), current);
    current = next;
  }

  // Step: Search for the leaf node by name
  std::cout << "Step: Search for the leaf node \"node_999\"" << std::endl;
  auto leaf = traversal::findByName(root, "node_" + std::to_string(depth - 1));

  // Assertion: Check if leaf was found
  std::cout << "Assertion: Check if leaf node was found" << std::endl;
  ASSERT_NE(leaf, nullptr);

  // Assertion: Check if name matches
  std::cout << "Assertion: Check if leaf node name is \"node_999\""
            << std::endl;
  EXPECT_EQ(leaf->getName(), "node_999");
}

TEST(NamedObjectStress, WideHierarchy) {
  // Step: Create wide hierarchy (5000 children)
  std::cout << "Step: Create wide hierarchy (5000 children)" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  int width = 5000;

  for (int i = 0; i < width; ++i) {
    NamedObject::create("child_" + std::to_string(i), root);
  }

  // Assertion: Check if number of children matches
  std::cout << "Assertion: Check if number of children is 5000" << std::endl;
  EXPECT_EQ(root->getChildren().size(), width);

  // Step: Search for the last child node by name
  std::cout << "Step: Search for the last child node \"child_4999\""
            << std::endl;
  auto child = traversal::findByName(root, "child_4999");

  // Assertion: Check if child was found
  std::cout << "Assertion: Check if child node was found" << std::endl;
  ASSERT_NE(child, nullptr);

  // Assertion: Check if name matches
  std::cout << "Assertion: Check if child node name is \"child_4999\""
            << std::endl;
  EXPECT_EQ(child->getName(), "child_4999");
}
