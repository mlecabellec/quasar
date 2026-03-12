#include <gtest/gtest.h>
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBuffer.hpp"

using namespace quasar::named;
using namespace quasar::named::traversal;

class TransformerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(TransformerTest, BasicOutwardTransformation) {
    auto root = NamedInteger<int>::create("root", 0);
    auto child1 = NamedString::create("child1", "hello");
    auto child2 = NamedInteger<int>::create("child2", 42);
    child1->setParent(root);
    child2->setParent(root);

    Transformer transformer;

    // Rule 1: Match "child1" and duplicate it, changing value
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child1";
        },
        [](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            auto original = ctx.getNode()->as<NamedString>();
            // emit two nodes
            auto mod1 = NamedString::create("child1_a", original->toString() + "_a");
            auto mod2 = NamedString::create("child1_b", original->toString() + "_b");
            return {mod1, mod2};
        },
        10 // priority
    );

    // Rule 2: Match "child2" and drop it (return empty)
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child2";
        },
        [](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            return {};
        },
        10 // priority
    );

    auto resultVec = transformer.transform(root);
    ASSERT_EQ(resultVec.size(), 1);
    auto transformedRoot = resultVec[0];

    EXPECT_EQ(transformedRoot->getName(), "root");
    EXPECT_NE(transformedRoot.get(), root.get()); // deep cloned

    auto children = transformedRoot->getChildren();
    ASSERT_EQ(children.size(), 2); // child1_a, child1_b. child2 is dropped.

    auto it = children.begin();
    EXPECT_EQ((*it)->getName(), "child1_a");
    EXPECT_EQ((*it)->as<NamedString>()->toString(), "hello_a");

    ++it;
    EXPECT_EQ((*it)->getName(), "child1_b");
    EXPECT_EQ((*it)->as<NamedString>()->toString(), "hello_b");
}

TEST_F(TransformerTest, InPlaceTransformation) {
    auto root = NamedInteger<int>::create("root", 0);
    auto child1 = NamedString::create("child1", "hello");
    auto child2 = NamedInteger<int>::create("child2", 42);
    child1->setParent(root);
    child2->setParent(root);

    Transformer transformer;

    // Replace child1 with multiple nodes in place
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child1";
        },
        [](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            auto mod1 = NamedString::create("child1_a", "x");
            auto mod2 = NamedString::create("child1_b", "y");
            return {mod1, mod2};
        }
    );

    transformer.transformInPlace(root);

    // Root is still root
    EXPECT_EQ(root->getName(), "root");
    
    // Check children
    auto children = root->getChildren();
    ASSERT_EQ(children.size(), 3); // child1_a, child1_b, child2

    auto child1a = root->getChild("child1_a");
    ASSERT_NE(child1a, nullptr);
    EXPECT_EQ(child1a->as<NamedString>()->toString(), "x");

    auto child1b = root->getChild("child1_b");
    ASSERT_NE(child1b, nullptr);
    EXPECT_EQ(child1b->as<NamedString>()->toString(), "y");

    auto child2Check = root->getChild("child2");
    ASSERT_NE(child2Check, nullptr);
    EXPECT_EQ(child2Check->as<NamedInteger<int>>()->value(), 42);
}

TEST_F(TransformerTest, ComplexTreeTransformation) {
    auto root = NamedObject::create("root");
    auto level1_a = NamedInteger<int>::create("l1_a", 10);
    auto level1_b = NamedString::create("l1_b", "hello");
    auto level2_a = NamedInteger<int>::create("l2_a", 20);
    auto level2_b = NamedInteger<int>::create("l2_b", 30);
    
    level1_a->setParent(root);
    level1_b->setParent(root);
    level2_a->setParent(level1_a);
    level2_b->setParent(level1_b);

    Transformer transformer;

    // Rule 1: multiply all NamedInteger values by 2
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->as<NamedInteger<int>>() != nullptr;
        },
        [&transformer](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            auto intNode = ctx.getNode()->as<NamedInteger<int>>();
            auto newNode = NamedInteger<int>::create(intNode->getName() + "_mult", intNode->value() * 2);
            
            // recursively process children manually because we're overriding the default behavior
            for(const auto& child : intNode->getChildren()) {
                auto transformedChild = transformer.transform(child);
                for(const auto& tc : transformedChild) {
                    tc->setParent(newNode);
                }
            }
            return {newNode};
        },
        5
    );

    // Rule 2: drop "l1_b" entirely including its children
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "l1_b";
        },
        [](const TransformContext& ctx) -> std::vector<std::shared_ptr<NamedObject>> {
            return {};
        },
        10
    );

    auto result = transformer.transform(root);
    ASSERT_EQ(result.size(), 1);
    auto newRoot = result[0];

    // root -> l1_a_mult -> l2_a_mult
    // l1_b is dropped, so l2_b is dropped too

    auto children = newRoot->getChildren();
    ASSERT_EQ(children.size(), 1);

    auto new_l1_a = children.front()->as<NamedInteger<int>>();
    ASSERT_NE(new_l1_a, nullptr);
    EXPECT_EQ(new_l1_a->getName(), "l1_a_mult");
    EXPECT_EQ(new_l1_a->value(), 20);

    auto l1_a_children = new_l1_a->getChildren();
    ASSERT_EQ(l1_a_children.size(), 1);

    auto new_l2_a = l1_a_children.front()->as<NamedInteger<int>>();
    ASSERT_NE(new_l2_a, nullptr);
    EXPECT_EQ(new_l2_a->getName(), "l2_a_mult");
    EXPECT_EQ(new_l2_a->value(), 40);
}
