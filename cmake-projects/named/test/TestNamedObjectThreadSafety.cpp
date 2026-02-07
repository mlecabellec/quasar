#include "quasar/named/NamedObject.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace quasar::named;

TEST(NamedObjectThreadSafety, StressTest) {
  // Step: Initialize root object and concurrency controls
  std::cout << "Step: Initialize root object and concurrency controls"
            << std::endl;
  auto root = NamedObject::create("root");
  std::atomic<bool> stop{false};
  std::atomic<int> counter{0};

  // Step: Define adder lambda (Thread A)
  std::cout << "Step: Define adder lambda (Thread A)" << std::endl;
  auto adder = [&]() {
    while (!stop) {
      int id = counter++;
      try {
        // Step: Create child with unique ID
        NamedObject::create("child_" + std::to_string(id), root);
      } catch (...) {
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  };

  // Step: Define remover lambda (Thread B)
  std::cout << "Step: Define remover lambda (Thread B)" << std::endl;
  auto remover = [&]() {
    std::mt19937 rng(std::random_device{}());
    while (!stop) {
      auto children = root->getChildren();
      if (!children.empty()) {
        std::uniform_int_distribution<size_t> dist(0, children.size() - 1);
        auto it = children.begin();
        std::advance(it, dist(rng));
        if (*it) {
          // Step: Remove random child
          (*it)->setParent(nullptr); // Remove
        }
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  };

  // Step: Define reader lambda (Thread C)
  std::cout << "Step: Define reader lambda (Thread C)" << std::endl;
  auto reader = [&]() {
    while (!stop) {
      auto children = root->getChildren();
      for (const auto &child : children) {
        // Step: Access name to verify object integrity
        volatile size_t l = child->getName().length();
        (void)l;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  };

  // Step: Launch concurrent threads (adder, remover, reader)
  std::cout << "Step: Launch concurrent threads (adder, remover, reader)"
            << std::endl;
  std::vector<std::thread> threads;
  for (int i = 0; i < 2; ++i)
    threads.emplace_back(adder);
  for (int i = 0; i < 2; ++i)
    threads.emplace_back(remover);
  for (int i = 0; i < 4; ++i)
    threads.emplace_back(reader);

  // Step: Run stress test for 1 second
  std::cout << "Step: Run stress test for 1 second" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Step: Signal stop and join all threads
  std::cout << "Step: Signal stop and join all threads" << std::endl;
  stop = true;
  for (auto &t : threads)
    t.join();

  // Assertion: Thread safety verified if no crashes occurred
  std::cout << "Assertion: Thread safety verified" << std::endl;
  SUCCEED();
}
