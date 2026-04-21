#include <gtest/gtest.h>
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/NamedObject.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <random>

using namespace quasar::named;
using namespace quasar::named::traversal;

class TransformerStressTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(TransformerStressTest, ConcurrentInPlaceTransformation) {
    // Create a large shared tree
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::vector<std::shared_ptr<NamedObject>> level1;
    for (int i = 0; i < 10; ++i) {
        level1.push_back(NamedObject::create("l1_" + std::to_string(i), root));
    }

    for (std::shared_ptr<NamedObject>& l1 : level1) {
        for (int j = 0; j < 50; ++j) {
            std::shared_ptr<NamedObject> l2 = NamedObject::create("l2_" + std::to_string(j), l1);
            for (int k = 0; k < 10; ++k) {
                NamedObject::create("l3_" + std::to_string(k), l2);
            }
        }
    }

    // Rules that do some renaming
    Transformer transformer;
    transformer.addRule(
        [](const TransformContext& ctx) { return true; }, // Match everything
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)t;
            std::shared_ptr<NamedObject> node = ctx.getNode();
            // Just rename the node to something else
            node->setName(node->getName() + "_visited");
            return {node};
        }
    );

    const int numThreads = 10; // Match 10 level1 nodes
    std::vector<std::thread> threads;
    std::atomic<bool> failed{false};

    for (int i = 0; i < numThreads; ++i) {
        std::shared_ptr<NamedObject> subtree = level1[i];
        threads.emplace_back([&transformer, subtree, &failed]() {
            try {
                // Perform in-place transformation on a specific subtree
                transformer.transformInPlace(subtree);
            } catch (const std::exception& e) {
                failed = true;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    ASSERT_FALSE(failed);
    // Basic structural check
    ASSERT_EQ(root->getChildren().size(), 10);
}

TEST_F(TransformerStressTest, PerformanceBenchmark) {
    // 100k nodes
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    for (int i = 0; i < 100; ++i) {
        std::shared_ptr<NamedObject> l1 = NamedObject::create("l1_" + std::to_string(i), root);
        for (int j = 0; j < 1000; ++j) {
            NamedObject::create("l2_" + std::to_string(j), l1);
        }
    }

    Transformer transformer;
    transformer.addRule(
        [](const TransformContext& ctx) { return ctx.getDepth() == 2; },
        [](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
            (void)t;
            std::shared_ptr<NamedObject> node = ctx.getNode();
            node->setName(node->getName() + "_leaf");
            return {node};
        }
    );

    auto start = std::chrono::high_resolution_clock::now();
    transformer.transformInPlace(root);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "[INFO] Transformed 100k nodes in-place in " << duration.count() << "ms" << std::endl;
}
