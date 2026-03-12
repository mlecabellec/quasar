#include <gtest/gtest.h>
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include <fstream>

namespace quasar::scripting {

class LuaServiceTest : public ::testing::Test {
protected:
    std::string m_scriptPath;

    void SetUp() override {
        m_scriptPath = std::string("/tmp/test_service_") + 
                       ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".lua";
        // Create a temporary Lua script for testing
        std::ofstream ofs(m_scriptPath);
        ofs << R"(
            service = {
                init_called = false,
                update_count = 0,
                shutdown_called = false
            }

            function service:onInit()
                self.init_called = true
                return true
            end

            function service:onUpdate(dt)
                self.update_count = self.update_count + 1
            end

            function service:onShutdown()
                self.shutdown_called = true
            end

            return service
        )";
        ofs.close();
    }

    void TearDown() override {
        std::remove(m_scriptPath.c_str());
    }
};

TEST_F(LuaServiceTest, FullLifecycle) {
    std::shared_ptr<LuaService> svc = LuaService::create("TestSvc");
    
    ASSERT_TRUE(svc->loadScript(m_scriptPath));
    
    // Test onInit
    EXPECT_TRUE(svc->onInit());
    
    // Hardening: stressors calling Lua from multiple threads concurrently
    std::atomic<bool> stopStress(false);
    std::vector<std::thread> stressors;
    for (int i = 0; i < 4; ++i) {
        stressors.emplace_back([svc, &stopStress]() {
            while (!stopStress) {
                svc->onUpdate(0.01);
                svc->execute("service.update_count = service.update_count + 1");
                std::this_thread::yield();
            }
        });
    }

    // Main thread also participates
    for (int i = 0; i < 100; ++i) {
        svc->onUpdate(0.1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    stopStress = true;
    for (auto& t : stressors) {
        if (t.joinable()) t.join();
    }
    
    // Check update_count in the service table. 
    sol::protected_function_result result = svc->execute("return service.update_count");
    EXPECT_GE(result.get<int>(), 100);

    svc->onShutdown();
}

TEST_F(LuaServiceTest, ObjectTracking) {
    std::shared_ptr<LuaService> svc = LuaService::create("TrackerSvc");
    LuaEngine& engine = *svc->getEngine();
    sol::state& lua = engine.getState();
    
    std::shared_ptr<named::NamedObject> obj = named::NamedObject::create("TrackMe");
    ObjectTracker::getInstance().track(obj);
    
    {
        std::unique_lock<std::recursive_timed_mutex> lock(svc->getStateMutex(), named::config::DEFAULT_LOCK_TIMEOUT);
        ASSERT_TRUE(lock.owns_lock());
        lua["trackedObj"] = LuaProxy<named::NamedObject>(obj);
        lua.script(R"(
            alive_before = quasar.isAlive(trackedObj)
        )");
        EXPECT_TRUE(lua["alive_before"].get<bool>());
    }
    
    // Now simulate object deletion from C++ side (though it's a shared_ptr, we only have weak_ptr in tracker)
    // To truely test 'isAlive' as 'is in hierarchy', we'd need isAlive to check parents.
    // Current implementation checks weak_ptr. Since 'obj' is still held by GTest and Lua, it's alive.
    
    // Let's test cleanup
    EXPECT_GT(ObjectTracker::getInstance().getTrackedCount(), 0);
    obj.reset();
    {
        std::lock_guard<std::recursive_mutex> lock(svc->getStateMutex());
        lua["trackedObj"] = sol::nil;
        lua.collect_garbage();
    }
    
    ObjectTracker::getInstance().cleanup();
    EXPECT_EQ(ObjectTracker::getInstance().getTrackedCount(), 0);
}

} // namespace quasar::scripting
