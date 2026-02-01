#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Traversal.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace quasar::named;

TEST(NamedObjectStress, DeepHierarchy) {
  // Test 1000 levels deep
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedObject> current = root;

  int depth = 1000;
  for (int i = 0; i < depth; ++i) {
    auto next = NamedObject::create("node_" + std::to_string(i), current);
    current = next;
  }

  // Verify finding the leaf
  auto leaf = traversal::findByName(
      root,
      "node_" + std::to_string(depth - 1)); // BFS/DFS might start from root
  ASSERT_NE(leaf, nullptr);
  EXPECT_EQ(leaf->getName(), "node_999");
}

TEST(NamedObjectStress, WideHierarchy) {
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  int width = 5000;

  for (int i = 0; i < width; ++i) {
    NamedObject::create("child_" + std::to_string(i), root);
  }

  EXPECT_EQ(root->getChildren().size(), width);

  auto child = traversal::findByName(root, "child_4999");
  ASSERT_NE(child, nullptr);
  EXPECT_EQ(child->getName(), "child_4999");
}
