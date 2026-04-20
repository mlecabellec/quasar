#include <gtest/gtest.h>
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <memory>
#include <vector>

using namespace quasar::named;
using namespace quasar::named::traversal;

class TransformerTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(TransformerTest, BasicOutwardTransformation) {
    std::shared_ptr<NamedString> root = NamedString::create("root", "root_val");
    std::shared_ptr<NamedString> child1 = NamedString::create("child1", "c1_val", root);
    std::shared_ptr<NamedString> child2 = NamedString::create("child2", "c2_val", root);

    Transformer transformer;

    // Rule 1: Transform child1 into two nodes
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child1";
        },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)t;
            std::shared_ptr<NamedString> original = std::dynamic_pointer_cast<NamedString>(ctx.getNode());
            // emit two nodes
            std::shared_ptr<NamedString> mod1 = NamedString::create("child1_a", original->toString() + "_a");
            std::shared_ptr<NamedString> mod2 = NamedString::create("child1_b", original->toString() + "_b");
            return {mod1, mod2};
        },
        10 // priority
    );

    // Rule 2: Delete child2
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child2";
        },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)ctx;
            (void)t;
            return {};
        },
        10 // priority
    );

    std::vector<std::shared_ptr<NamedObject>> result = transformer.transform(root);

    ASSERT_EQ(result.size(), 1);
    std::shared_ptr<NamedObject> newRoot = result[0];
    ASSERT_EQ(newRoot->getName(), "root");
    
    std::list<std::shared_ptr<NamedObject>> children = newRoot->getChildren();
    ASSERT_EQ(children.size(), 2);
    
    bool foundA = false;
    bool foundB = false;
    for (const std::shared_ptr<NamedObject>& child : children) {
        if (child->getName() == "child1_a") foundA = true;
        if (child->getName() == "child1_b") foundB = true;
    }
    ASSERT_TRUE(foundA);
    ASSERT_TRUE(foundB);
}

TEST_F(TransformerTest, InPlaceTransformation) {
    std::shared_ptr<NamedString> root = NamedString::create("root", "v");
    std::shared_ptr<NamedString> child1 = NamedString::create("child1", "v1", root);
    std::shared_ptr<NamedString> child2 = NamedString::create("child2", "v2", root);

    Transformer transformer;

    // Rule: Replace child1 with two strings
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "child1";
        },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)ctx;
            (void)t;
            std::shared_ptr<NamedString> mod1 = NamedString::create("child1_a", "x");
            std::shared_ptr<NamedString> mod2 = NamedString::create("child1_b", "y");
            return {mod1, mod2};
        }
    );

    transformer.transformInPlace(root);

    ASSERT_EQ(root->getChildren().size(), 3); // child1_a, child1_b, child2
    
    bool foundA = false;
    bool foundB = false;
    bool found2 = false;
    for (const std::shared_ptr<NamedObject>& child : root->getChildren()) {
        if (child->getName() == "child1_a") foundA = true;
        if (child->getName() == "child1_b") foundB = true;
        if (child->getName() == "child2") found2 = true;
    }
    ASSERT_TRUE(foundA);
    ASSERT_TRUE(foundB);
    ASSERT_TRUE(found2);
}

TEST_F(TransformerTest, ComplexTreeTransformation) {
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::shared_ptr<NamedObject> l1_a = NamedObject::create("l1_a", root);
    std::shared_ptr<NamedObject> l1_b = NamedObject::create("l1_b", root);
    
    std::shared_ptr<NamedInteger<int>> val1 = NamedInteger<int>::create("val1", 10, l1_a);
    std::shared_ptr<NamedInteger<int>> val2 = NamedInteger<int>::create("val2", 20, l1_b);

    Transformer transformer;

    // Rule: multiply all integers by 2 and rename
    transformer.addRule(
        [](const TransformContext& ctx) {
            return std::dynamic_pointer_cast<NamedInteger<int>>(ctx.getNode()) != nullptr;
        },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            std::shared_ptr<NamedInteger<int>> intNode = std::dynamic_pointer_cast<NamedInteger<int>>(ctx.getNode());
            std::shared_ptr<NamedInteger<int>> newNode = NamedInteger<int>::create(intNode->getName() + "_mult", intNode->value() * 2);

            // recursively process children manually because we're overriding the default behavior
            for(const std::shared_ptr<NamedObject>& child : intNode->getChildren()) {
                std::vector<std::shared_ptr<NamedObject>> transformedChild = t.transformSubtree(child, ctx.getDepth() + 1, ctx.getPath() + "/" + child->getName());
                for(const std::shared_ptr<NamedObject>& tc : transformedChild) {
                    tc->setParent(newNode);
                }
            }
            return {newNode};
        },
        5
    );

    // Rule: Delete l1_b
    transformer.addRule(
        [](const TransformContext& ctx) {
            return ctx.getNode()->getName() == "l1_b";
        },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)ctx;
            (void)t;
            return {};
        },
        10
    );

    std::vector<std::shared_ptr<NamedObject>> result = transformer.transform(root);
    ASSERT_EQ(result.size(), 1);
    std::shared_ptr<NamedObject> newRoot = result[0];

    // l1_b should be gone, l1_a should be there
    ASSERT_EQ(newRoot->getChildren().size(), 1);
    std::shared_ptr<NamedObject> newL1A = newRoot->getFirstChild();
    ASSERT_EQ(newL1A->getName(), "l1_a");

    // val1 should be val1_mult with value 20
    ASSERT_EQ(newL1A->getChildren().size(), 1);
    std::shared_ptr<NamedInteger<int>> newVal1 = std::dynamic_pointer_cast<NamedInteger<int>>(newL1A->getFirstChild());
    ASSERT_NE(newVal1, nullptr);
    ASSERT_EQ(newVal1->getName(), "val1_mult");
    ASSERT_EQ(newVal1->value(), 20);
}
