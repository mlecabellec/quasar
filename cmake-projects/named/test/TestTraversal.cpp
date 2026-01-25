#include <gtest/gtest.h>
#include "quasar/named/NamedObject.h"
#include "quasar/named/Traversal.h"
#include "quasar/named/NamedInteger.h"

using namespace quasar::named;

TEST(TraversalTest, DFS) {
    auto root = NamedObject::create("root");
    auto c1 = NamedObject::create("c1", root);
    auto c2 = NamedObject::create("c2", root);
    auto c11 = NamedObject::create("c11", c1);
    
    std::vector<std::string> visited;
    traversal::forEachDepthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
        visited.push_back(obj->getName());
    });
    
    // Order: root, c1, c11, c2
    ASSERT_EQ(visited.size(), 4);
    EXPECT_EQ(visited[0], "root");
    EXPECT_EQ(visited[1], "c1");
    EXPECT_EQ(visited[2], "c11");
    EXPECT_EQ(visited[3], "c2");
}

TEST(TraversalTest, BFS) {
    auto root = NamedObject::create("root");
    auto c1 = NamedObject::create("c1", root);
    auto c2 = NamedObject::create("c2", root);
    auto c11 = NamedObject::create("c11", c1);
    
    std::vector<std::string> visited;
    traversal::forEachBreadthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
        visited.push_back(obj->getName());
    });
    
    // Order: root, c1, c2, c11
    ASSERT_EQ(visited.size(), 4);
    EXPECT_EQ(visited[0], "root");
    EXPECT_EQ(visited[1], "c1");
    EXPECT_EQ(visited[2], "c2");
    EXPECT_EQ(visited[3], "c11");
}

TEST(TraversalTest, FindByName) {
    auto root = NamedObject::create("root");
    auto c1 = NamedObject::create("c1", root);
    
    auto found = traversal::findByName(root, "c1");
    EXPECT_EQ(found, c1);
    
    EXPECT_EQ(traversal::findByName(root, "missing"), nullptr);
}

TEST(TraversalTest, DeepCopy) {
    auto root = NamedObject::create("root");
    auto c1 = NamedInteger<int>::create("c1", 42, root);
    
    auto copy = traversal::deepCopy(root);
    EXPECT_NE(copy, root);
    EXPECT_EQ(copy->getName(), "root");
    EXPECT_EQ(copy->getChildren().size(), 1);
    
    auto childCopy = copy->getFirstChild();
    EXPECT_NE(childCopy, c1);
    EXPECT_EQ(childCopy->getName(), "c1");
    
    // Check type preservation
    auto intCopy = std::dynamic_pointer_cast<NamedInteger<int>>(childCopy);
    ASSERT_TRUE(intCopy != nullptr);
    EXPECT_EQ(intCopy->toInt(), 42);
    
    // Modify copy, ensure original unchanged
    auto c2 = NamedObject::create("c2", copy);
    EXPECT_EQ(copy->getChildren().size(), 2);
    EXPECT_EQ(root->getChildren().size(), 1);
}
