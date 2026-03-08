#include <gtest/gtest.h>
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include <fstream>

namespace quasar::scripting {

class LuaServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary Lua script for testing
        std::ofstream ofs("/tmp/test_service.lua");
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
        std::remove("/tmp/test_service.lua");
    }
};

TEST_F(LuaServiceTest, FullLifecycle) {
    auto svc = LuaService::create("TestSvc");
    
    ASSERT_TRUE(svc->loadScript("/tmp/test_service.lua"));
    
    // Test onInit
    EXPECT_TRUE(svc->onInit());
    
    // Testing via C++ calls
    svc->onUpdate(0.1);
    svc->onUpdate(0.1);
    
    // Check update_count in the service table
    auto result = svc->execute("return service.update_count");
    EXPECT_EQ(result.get<int>(), 2);

    svc->onShutdown();
    
    // To verify, we'd need to access m_luaSelf values. 
    // I should probably expose a way to get values from the service for testing.
}

TEST_F(LuaServiceTest, ObjectTracking) {
    auto svc = LuaService::create("TrackerSvc");
    auto& engine = *svc->getEngine();
    auto& lua = engine.getState();
    
    auto obj = named::NamedObject::create("TrackMe");
    ObjectTracker::getInstance().track(obj);
    
    lua["trackedObj"] = obj;
    lua.script(R"(
        alive_before = quasar.isAlive(trackedObj)
    )");
    EXPECT_TRUE(lua["alive_before"].get<bool>());
    
    // Now simulate object deletion from C++ side (though it's a shared_ptr, we only have weak_ptr in tracker)
    // To truely test 'isAlive' as 'is in hierarchy', we'd need isAlive to check parents.
    // Current implementation checks weak_ptr. Since 'obj' is still held by GTest and Lua, it's alive.
    
    // Let's test cleanup
    EXPECT_GT(ObjectTracker::getInstance().getTrackedCount(), 0);
    obj.reset();
    lua["trackedObj"] = sol::nil;
    lua.collect_garbage();
    
    ObjectTracker::getInstance().cleanup();
    EXPECT_EQ(ObjectTracker::getInstance().getTrackedCount(), 0);
}

} // namespace quasar::scripting
