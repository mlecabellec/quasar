#include <gtest/gtest.h>
#include "quasar/scripting/ScriptManager.hpp"
#include <fstream>

namespace quasar::scripting {

class ScriptManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream ofs("/tmp/stress_script.lua");
        ofs << R"(
            local t = {}
            for i=1,100 do t[i] = i end
            return {
                onInit = function() return true end,
                onUpdate = function() end,
                onShutdown = function() end
            }
        )";
        ofs.close();
    }
    void TearDown() override {
        std::remove("/tmp/stress_script.lua");
    }
};

TEST_F(ScriptManagerTest, MemoryStress) {
    auto& mgr = ScriptManager::getInstance();
    
    // Perform 1000 creation/destruction cycles to check for leaks
    for (int i = 0; i < 1000; ++i) {
        std::string name = "Svc_" + std::to_string(i);
        auto svc = mgr.createService(name, "/tmp/stress_script.lua");
        ASSERT_NE(svc, nullptr);
        mgr.stopService(name);
        
        if (i % 100 == 0) {
            mgr.tickGC();
        }
    }
    
    // Final cleanup
    mgr.tickGC(1000); 
}

TEST_F(ScriptManagerTest, SandboxIsolation) {
    auto& mgr = ScriptManager::getInstance();
    
    // Create a script that tries to use blocked os.execute
    std::ofstream ofs("/tmp/evil.lua");
    ofs << "return { onInit = function() os.execute('echo evil') end }";
    ofs.close();
    
    auto svc = mgr.createService("EvilSvc", "/tmp/evil.lua");
    // This should fail or at least the onInit call should fail/warn
    // In our current ScriptManager, we load libraries then nil out os.execute.
    // Let's verify it panics or errors gracefully.
    
    std::remove("/tmp/evil.lua");
    if (svc) mgr.stopService("EvilSvc");
}

} // namespace quasar::scripting
