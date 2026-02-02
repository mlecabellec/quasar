#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include "quasar/named/NamedObject.hpp"

using namespace quasar::named;

TEST(NamedObjectThreadSafety, StressTest) {
    auto root = NamedObject::create("root");
    std::atomic<bool> stop{false};
    std::atomic<int> counter{0};
    
    // Thread A: Add children
    auto adder = [&]() {
        while (!stop) {
            int id = counter++;
            try {
                NamedObject::create("child_" + std::to_string(id), root);
            } catch (...) {}
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };
    
    // Thread B: Remove children
    auto remover = [&]() {
        std::mt19937 rng(std::random_device{}());
        while (!stop) {
            auto children = root->getChildren();
            if (!children.empty()) {
                std::uniform_int_distribution<size_t> dist(0, children.size() - 1);
                auto it = children.begin();
                std::advance(it, dist(rng));
                if (*it) {
                    (*it)->setParent(nullptr); // Remove
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };
    
    // Thread C: Reader
    auto reader = [&]() {
        while (!stop) {
            auto children = root->getChildren();
            for (const auto& child : children) {
                // Access name to verify object integrity
                volatile size_t l = child->getName().length();
                (void)l;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };
    
    std::vector<std::thread> threads;
    for(int i=0; i<2; ++i) threads.emplace_back(adder);
    for(int i=0; i<2; ++i) threads.emplace_back(remover);
    for(int i=0; i<4; ++i) threads.emplace_back(reader);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    stop = true;
    for(auto& t : threads) t.join();
    
    // Cleanup check
    // root->getChildren() should be valid
    SUCCEED();
}
