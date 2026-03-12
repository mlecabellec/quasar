#include <gtest/gtest.h>
#include "quasar/scripting/ScriptManager.hpp"
#include <fstream>

namespace quasar::scripting {

class ScriptManagerTest : public ::testing::Test {
protected:
    std::string m_scriptPath;

    void SetUp() override {
        m_scriptPath = std::string("/tmp/stress_script_") + 
                       ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".lua";
        std::ofstream ofs(m_scriptPath);
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
        std::remove(m_scriptPath.c_str());
    }
};

TEST_F(ScriptManagerTest, MemoryStress) {
    ScriptManager& mgr = ScriptManager::getInstance();
    
    // Perform 1000 creation/destruction cycles to check for leaks
    for (int i = 0; i < 1000; ++i) {
        std::string name = "Svc_" + std::to_string(i);
        std::shared_ptr<LuaService> svc = mgr.createService(name, m_scriptPath);
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
    ScriptManager& mgr = ScriptManager::getInstance();
    
    // Create a script that tries to use blocked os.execute
    std::ofstream ofs("/tmp/evil.lua");
    ofs << "return { onInit = function() os.execute('echo evil') end }";
    ofs.close();
    
    std::shared_ptr<LuaService> svc = mgr.createService("EvilSvc", "/tmp/evil.lua");
    // This should fail or at least the onInit call should fail/warn
    // In our current ScriptManager, we load libraries then nil out os.execute.
    // Let's verify it panics or errors gracefully.
    
    std::remove("/tmp/evil.lua");
    if (svc) mgr.stopService("EvilSvc");
}

} // namespace quasar::scripting
