#include <gtest/gtest.h>
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include <thread>
#include <chrono>

namespace quasar::scripting {

using namespace quasar::named;

class LuaServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = NamedObject::create("root");
    }

    std::shared_ptr<NamedObject> root;
};

TEST_F(LuaServiceTest, BasicLifecycle) {
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();
    EXPECT_TRUE(service->isRunning());

    std::atomic<bool> executed{false};
    service->postTask([&executed]() {
        executed = true;
    });

    // Wait for task
    for (int i = 0; i < 100 && !executed; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(executed);

    service->stop();
    EXPECT_FALSE(service->isRunning());
}

TEST_F(LuaServiceTest, ScriptExecution) {
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    // execute() is synchronous and returns sol::protected_function_result
    sol::protected_function_result result = service->execute("return 10 + 20");
    
    ASSERT_TRUE(result.valid());
    int value = result.get<int>();
    EXPECT_EQ(value, 30);

    service->stop();
}

TEST_F(LuaServiceTest, ObjectTracking) {
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    // Create an object that is NOT in the tree, but tracked by Lua
    sol::protected_function_result result = service->execute(R"(
        local obj = quasar.named.createObject("Standalone")
        return obj
    )");
    
    ASSERT_TRUE(result.valid());
    
    // Standalone object should be kept alive by ObjectTracker (strong ref)
    sol::object obj = result.get<sol::object>();
    std::shared_ptr<NamedObject> standalone = extractNamedObject(obj);
    ASSERT_NE(standalone, nullptr);
    EXPECT_EQ(standalone->getName(), "Standalone");

    service->stop();
    
    // After service stop and engine shutdown, the strong reference should be gone.
    standalone.reset();
}

TEST_F(LuaServiceTest, ErrorHandling) {
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    sol::protected_function_result result = service->execute("error('test error')");
    
    EXPECT_FALSE(result.valid());
    sol::error err = result;
    EXPECT_TRUE(std::string(err.what()).find("test error") != std::string::npos);

    service->stop();
}

TEST_F(LuaServiceTest, WorstCaseTermination) {
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    // Start a long-running execution inside a separate thread
    std::thread t([&]() {
        service->execute("for i=1,10000000 do end");
    });

    // Abrupt termination while Lua is actively running
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    service->stop();
    service.reset();

    t.join();
    EXPECT_TRUE(true); // If we reached here without a segfault, test passed
}

} // namespace quasar::scripting
