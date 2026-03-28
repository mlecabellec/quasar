#include <gtest/gtest.h>
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <thread>
#include <chrono>
#include <atomic>

using namespace quasar::named;

class NamedMethodAndServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = NamedObject::create("root");
    }

    std::shared_ptr<NamedObject> root;
};

TEST_F(NamedMethodAndServiceTest, NamedMethodBasic) {
    auto method = NamedMethod::create("testMethod", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        auto val = std::dynamic_pointer_cast<NamedInteger<int>>(args);
        int input = val ? val->value() : 0;
        return NamedInteger<int>::create("result", input * 2);
    }, root);

    auto args = NamedInteger<int>::create("args", 21);
    auto result = method->execute(args);
    auto resultInt = std::dynamic_pointer_cast<NamedInteger<int>>(result);

    ASSERT_NE(resultInt, nullptr);
    EXPECT_EQ(resultInt->value(), 42);
}

TEST_F(NamedMethodAndServiceTest, NamedMethodOwnerAccess) {
    auto data = NamedInteger<int>::create("data", 10, root);
    auto method = NamedMethod::create("increment", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        auto ownerData = std::dynamic_pointer_cast<NamedInteger<int>>(owner->getChild("data"));
        if (ownerData) {
            ownerData->setValue(ownerData->value() + 1);
        }
        return nullptr;
    }, root);

    method->execute(nullptr);
    EXPECT_EQ(data->value(), 11);
}

TEST_F(NamedMethodAndServiceTest, NamedServiceLifecycle) {
    auto service = NamedService::create("testService", root);
    std::atomic<int> counter{0};

    NamedMethod::create("run", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        counter++;
        return nullptr;
    }, service);

    service->setCycleTime(std::chrono::milliseconds(10));
    service->start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    service->stop();
    int finalCounter = counter.load();
    EXPECT_GE(finalCounter, 3);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_EQ(counter.load(), finalCounter);
}

TEST_F(NamedMethodAndServiceTest, NamedServiceHooks) {
    auto service = NamedService::create("hookService", root);
    bool configured = false;
    bool started = false;
    bool stopped = false;

    NamedMethod::create("configure", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        configured = true;
        return nullptr;
    }, service);

    NamedMethod::create("onStart", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        started = true;
        return nullptr;
    }, service);

    NamedMethod::create("onStop", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        stopped = true;
        return nullptr;
    }, service);

    service->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    service->stop();

    EXPECT_TRUE(configured);
    EXPECT_TRUE(started);
    EXPECT_TRUE(stopped);
}

TEST_F(NamedMethodAndServiceTest, MultiThreadStress) {
    const int NUM_SERVICES = 20;
    std::vector<std::shared_ptr<NamedService>> services;
    std::atomic<int> totalRuns{0};

    for (int i = 0; i < NUM_SERVICES; ++i) {
        auto service = NamedService::create("service_" + std::to_string(i), root);
        NamedMethod::create("run", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
            totalRuns++;
            return nullptr;
        }, service);
        service->setCycleTime(std::chrono::milliseconds(1));
        services.push_back(service);
    }

    // Start all
    for (auto& s : services) s->start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop all
    for (auto& s : services) s->stop();

    EXPECT_GT(totalRuns.load(), NUM_SERVICES * 10);
}
