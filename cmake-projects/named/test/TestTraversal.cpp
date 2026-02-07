#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Traversal.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;

TEST(TraversalTest, DFS) {
  // Step: Create tree for DFS traversal
  std::cout << "Step: Create tree for DFS traversal" << std::endl;
  auto root = NamedObject::create("root");
  auto c1 = NamedObject::create("c1", root);
  auto c2 = NamedObject::create("c2", root);
  auto c11 = NamedObject::create("c11", c1);

  // Step: Perform forEachDepthFirst
  std::cout << "Step: Perform forEachDepthFirst" << std::endl;
  std::vector<std::string> visited;
  traversal::forEachDepthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
    visited.push_back(obj->getName());
  });

  // Assertion: Check visited count
  std::cout << "Assertion: Check visited count is 4" << std::endl;
  ASSERT_EQ(visited.size(), 4);

  // Assertion: Check DFS order (root, c1, c11, c2)
  std::cout << "Assertion: Check DFS order (root, c1, c11, c2)" << std::endl;
  EXPECT_EQ(visited[0], "root");
  EXPECT_EQ(visited[1], "c1");
  EXPECT_EQ(visited[2], "c11");
  EXPECT_EQ(visited[3], "c2");
}

TEST(TraversalTest, BFS) {
  // Step: Create tree for BFS traversal
  std::cout << "Step: Create tree for BFS traversal" << std::endl;
  auto root = NamedObject::create("root");
  auto c1 = NamedObject::create("c1", root);
  auto c2 = NamedObject::create("c2", root);
  auto c11 = NamedObject::create("c11", c1);

  // Step: Perform forEachBreadthFirst
  std::cout << "Step: Perform forEachBreadthFirst" << std::endl;
  std::vector<std::string> visited;
  traversal::forEachBreadthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
    visited.push_back(obj->getName());
  });

  // Assertion: Check visited count
  std::cout << "Assertion: Check visited count is 4" << std::endl;
  ASSERT_EQ(visited.size(), 4);

  // Assertion: Check BFS order (root, c1, c2, c11)
  std::cout << "Assertion: Check BFS order (root, c1, c2, c11)" << std::endl;
  EXPECT_EQ(visited[0], "root");
  EXPECT_EQ(visited[1], "c1");
  EXPECT_EQ(visited[2], "c2");
  EXPECT_EQ(visited[3], "c11");
}

TEST(TraversalTest, FindByName) {
  // Step: Create tree for search
  std::cout << "Step: Create tree for search" << std::endl;
  auto root = NamedObject::create("root");
  auto c1 = NamedObject::create("c1", root);

  // Step: Search for "c1"
  std::cout << "Step: Search for \"c1\"" << std::endl;
  auto found = traversal::findByName(root, "c1");

  // Assertion: Check if "c1" was found
  std::cout << "Assertion: Check if \"c1\" was found" << std::endl;
  EXPECT_EQ(found, c1);

  // Step: Search for non-existent node
  std::cout << "Step: Search for \"missing\"" << std::endl;
  auto missing = traversal::findByName(root, "missing");

  // Assertion: Check if search returned nullptr
  std::cout << "Assertion: Check if search for \"missing\" returns nullptr"
            << std::endl;
  EXPECT_EQ(missing, nullptr);
}

TEST(TraversalTest, DeepCopy) {
  // Step: Create tree for deep copy
  std::cout << "Step: Create tree for deep copy" << std::endl;
  auto root = NamedObject::create("root");
  auto c1 = NamedInteger<int>::create("c1", 42, root);

  // Step: Perform deepCopy
  std::cout << "Step: Perform deepCopy" << std::endl;
  auto copy = traversal::deepCopy(root);

  // Assertion: Check if copy is different instance
  std::cout << "Assertion: Check if copy is a different instance" << std::endl;
  EXPECT_NE(copy, root);

  // Assertion: Check copy root name
  std::cout << "Assertion: Check if copy root name is \"root\"" << std::endl;
  EXPECT_EQ(copy->getName(), "root");

  // Assertion: Check copy children count
  std::cout << "Assertion: Check if copy has 1 child" << std::endl;
  EXPECT_EQ(copy->getChildren().size(), 1);

  // Step: Access child of copy
  std::cout << "Step: Access child of copy" << std::endl;
  auto childCopy = copy->getFirstChild();

  // Assertion: Check if child copy is different instance
  std::cout << "Assertion: Check if child copy is a different instance"
            << std::endl;
  EXPECT_NE(childCopy, c1);

  // Assertion: Check child copy name
  std::cout << "Assertion: Check if child copy name is \"c1\"" << std::endl;
  EXPECT_EQ(childCopy->getName(), "c1");

  // Step: Cast child copy to NamedInteger and check type preservation
  std::cout
      << "Step: Cast child copy to NamedInteger and check type preservation"
      << std::endl;
  auto intCopy = std::dynamic_pointer_cast<NamedInteger<int>>(childCopy);

  // Assertion: Check if cast was successful
  std::cout << "Assertion: Check if cast was successful" << std::endl;
  ASSERT_TRUE(intCopy != nullptr);

  // Assertion: Check if value was preserved
  std::cout << "Assertion: Check if value is 42" << std::endl;
  EXPECT_EQ(intCopy->toInt(), 42);

  // Step: Modify copy and ensure original is unchanged
  std::cout << "Step: Modify copy and ensure original is unchanged"
            << std::endl;
  auto c2 = NamedObject::create("c2", copy);

  // Assertion: Check copy children count
  std::cout << "Assertion: Check if copy children size is now 2" << std::endl;
  EXPECT_EQ(copy->getChildren().size(), 2);

  // Assertion: Check original children count
  std::cout << "Assertion: Check if original children size is still 1"
            << std::endl;
  EXPECT_EQ(root->getChildren().size(), 1);
}
