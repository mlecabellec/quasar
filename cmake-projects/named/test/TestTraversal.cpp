#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Traversal.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;

TEST(TraversalTest, DFS) {
  // Proof of compliance: [FE-0020.12.1] Depth-first iteration.
  // Step: Create tree for DFS traversal
  std::cout << "Step: Create tree for DFS traversal" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedObject> c1 = NamedObject::create("c1", root);
  std::shared_ptr<NamedObject> c2 = NamedObject::create("c2", root);
  std::shared_ptr<NamedObject> c11 = NamedObject::create("c11", c1);

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
  // Proof of compliance: [FE-0020.12.1] Breadth-first iteration.
  // Step: Create tree for BFS traversal
  std::cout << "Step: Create tree for BFS traversal" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedObject> c1 = NamedObject::create("c1", root);
  std::shared_ptr<NamedObject> c2 = NamedObject::create("c2", root);
  std::shared_ptr<NamedObject> c11 = NamedObject::create("c11", c1);

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
  // Proof of compliance: [FE-0020.13] Search the tree by name.
  // Step: Create tree for search
  std::cout << "Step: Create tree for search" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedObject> c1 = NamedObject::create("c1", root);

  // Step: Search for "c1"
  std::cout << "Step: Search for \"c1\"" << std::endl;
  std::shared_ptr<NamedObject> found = traversal::findByName(root, "c1");

  // Assertion: Check if "c1" was found
  std::cout << "Assertion: Check if \"c1\" was found" << std::endl;
  EXPECT_EQ(found, c1);

  // Step: Search for non-existent node
  std::cout << "Step: Search for \"missing\"" << std::endl;
  std::shared_ptr<NamedObject> missing = traversal::findByName(root, "missing");

  // Assertion: Check if search returned nullptr
  std::cout << "Assertion: Check if search for \"missing\" returns nullptr"
            << std::endl;
  EXPECT_EQ(missing, nullptr);
}

TEST(TraversalTest, DeepCopy) {
  // Proof of compliance: [FE-0020.14] deep copy.
  // Step: Create tree for deep copy
  std::cout << "Step: Create tree for deep copy" << std::endl;
  std::shared_ptr<NamedObject> root = NamedObject::create("root");
  std::shared_ptr<NamedInteger<int>> c1 =
      NamedInteger<int>::create("c1", 42, root);

  // Step: Perform deepCopy
  std::cout << "Step: Perform deepCopy" << std::endl;
  std::shared_ptr<NamedObject> copy = traversal::deepCopy(root);

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
  std::shared_ptr<NamedObject> childCopy = copy->getFirstChild();

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
  std::shared_ptr<NamedInteger<int>> intCopy =
      std::dynamic_pointer_cast<NamedInteger<int>>(childCopy);

  // Assertion: Check if cast was successful
  std::cout << "Assertion: Check if cast was successful" << std::endl;
  ASSERT_TRUE(intCopy != nullptr);

  // Assertion: Check if value was preserved
  std::cout << "Assertion: Check if value is 42" << std::endl;
  EXPECT_EQ(intCopy->toInt(), 42);

  // Step: Modify copy and ensure original is unchanged
  std::cout << "Step: Modify copy and ensure original is unchanged"
            << std::endl;
  std::shared_ptr<NamedObject> c2 = NamedObject::create("c2", copy);

  // Assertion: Check copy children count
  std::cout << "Assertion: Check if copy children size is now 2" << std::endl;
  EXPECT_EQ(copy->getChildren().size(), 2);

  // Assertion: Check original children count
  std::cout << "Assertion: Check if original children size is still 1"
            << std::endl;
  EXPECT_EQ(root->getChildren().size(), 1);
}
