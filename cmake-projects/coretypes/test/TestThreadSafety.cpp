#include "quasar/coretypes/FloatingPointTypes.hpp"
#include "quasar/coretypes/IntegerTypes.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace quasar::coretypes;

TEST(CoreTypesThreadSafety, IntegerImmutable) {
  // Step: Initialize shared Int and atomic stop flag
  std::cout << "Step: Initialize shared Int and atomic stop flag" << std::endl;
  Int sharedInt(42);
  std::atomic<bool> stop{false};

  // Step: Define reader lambda
  std::cout << "Step: Define reader lambda" << std::endl;
  auto reader = [&]() {
    while (!stop) {
      // Assertion: Check if sharedInt value is still 42
      EXPECT_EQ(sharedInt.toInt(), 42);
      // Step: Copy sharedInt
      Int copy = sharedInt;
      // Assertion: Check if copy value is 42
      EXPECT_EQ(copy.toInt(), 42);
    }
  };

  // Step: Launch 10 reader threads
  std::cout << "Step: Launch 10 reader threads" << std::endl;
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(reader);
  }

  // Step: Sleep for 100ms
  std::cout << "Step: Sleep for 100ms" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Step: Signal threads to stop and join
  std::cout << "Step: Signal threads to stop and join" << std::endl;
  stop = true;
  for (auto &t : threads)
    t.join();

  // Assertion: Thread safety verified if no race detected
  std::cout << "Assertion: Thread safety verified" << std::endl;
}

TEST(CoreTypesThreadSafety, FloatingPointImmutable) {
  // Step: Initialize shared Double and atomic stop flag
  std::cout << "Step: Initialize shared Double and atomic stop flag"
            << std::endl;
  Double sharedDouble(3.14);
  std::atomic<bool> stop{false};

  // Step: Define reader lambda
  std::cout << "Step: Define reader lambda" << std::endl;
  auto reader = [&]() {
    while (!stop) {
      // Assertion: Check if sharedDouble value is 3.14
      EXPECT_DOUBLE_EQ(sharedDouble.toDouble(), 3.14);
      // Step: Copy sharedDouble
      Double copy = sharedDouble;
      // Assertion: Check if copy value is 3.14
      EXPECT_DOUBLE_EQ(copy.toDouble(), 3.14);
    }
  };

  // Step: Launch 10 reader threads
  std::cout << "Step: Launch 10 reader threads" << std::endl;
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(reader);
  }

  // Step: Sleep for 100ms
  std::cout << "Step: Sleep for 100ms" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Step: Signal threads to stop and join
  std::cout << "Step: Signal threads to stop and join" << std::endl;
  stop = true;
  for (auto &t : threads)
    t.join();

  // Assertion: Thread safety verified
  std::cout << "Assertion: Thread safety verified" << std::endl;
}
