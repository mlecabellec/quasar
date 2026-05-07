/**
 * @file TestTransformerStress.cpp
 * @brief Stress and performance tests for the Transformer traversal engine.
 */

#include <gtest/gtest.h>
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/NamedObject.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <random>

using namespace quasar::named;
using namespace quasar::named::traversal;

/**
 * @class TransformerStressTest
 * @brief Test fixture for Transformer stress testing.
 */
class TransformerStressTest : public ::testing::Test {
protected:
    /**
     * @brief Setup the test fixture.
     */
    void SetUp() override {}
};

/**
 * @brief Verify concurrent in-place transformation on different subtrees.
 */
TEST_F(TransformerStressTest, ConcurrentInPlaceTransformation) {
    // Create a large shared tree
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::vector<std::shared_ptr<NamedObject>> level1;
    // [CS-0010.37] Loop hard limit.
    size_t count1 = 0;
    for (int i = 0; i < 10; ++i) {
        if (++count1 > 1000) throw std::runtime_error("Loop limit exceeded");
        level1.push_back(NamedObject::create("l1_" + std::to_string(i), root));
    }

    // [CS-0010.37] Loop hard limit.
    size_t count2 = 0;
    for (std::shared_ptr<NamedObject>& l1 : level1) {
        if (++count2 > 1000) throw std::runtime_error("Loop limit exceeded");
        // [CS-0010.37] Loop hard limit.
        size_t count3 = 0;
        for (int j = 0; j < 50; ++j) {
            if (++count3 > 1000) throw std::runtime_error("Loop limit exceeded");
            std::shared_ptr<NamedObject> l2 = NamedObject::create("l2_" + std::to_string(j), l1);
            // [CS-0010.37] Loop hard limit.
            size_t count4 = 0;
            for (int k = 0; k < 10; ++k) {
                if (++count4 > 1000) throw std::runtime_error("Loop limit exceeded");
                (void)NamedObject::create("l3_" + std::to_string(k), l2);
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

    // [CS-0010.37] Loop hard limit.
    size_t count5 = 0;
    for (int i = 0; i < numThreads; ++i) {
        if (++count5 > 1000) throw std::runtime_error("Loop limit exceeded");
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

    // [CS-0010.37] Loop hard limit.
    size_t count6 = 0;
    for (std::thread& t : threads) {
        if (++count6 > 1000) throw std::runtime_error("Loop limit exceeded");
        t.join();
    }

    ASSERT_FALSE(failed);
    // Basic structural check
    ASSERT_EQ(root->getChildren().size(), 10);
}

/**
 * @brief Benchmark in-place transformation on a 100k node tree.
 */
TEST_F(TransformerStressTest, PerformanceBenchmark) {
    // 100k nodes
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    // [CS-0010.37] Loop hard limit.
    size_t count1 = 0;
    for (int i = 0; i < 100; ++i) {
        if (++count1 > 1000) throw std::runtime_error("Loop limit exceeded");
        std::shared_ptr<NamedObject> l1 = NamedObject::create("l1_" + std::to_string(i), root);
        // [CS-0010.37] Loop hard limit.
        size_t count2 = 0;
        for (int j = 0; j < 1000; ++j) {
            if (++count2 > 100000) throw std::runtime_error("Loop limit exceeded");
            (void)NamedObject::create("l2_" + std::to_string(j), l1);
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

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    transformer.transformInPlace(root);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "[INFO] Transformed 100k nodes in-place in " << duration.count() << "ms" << std::endl;
}
