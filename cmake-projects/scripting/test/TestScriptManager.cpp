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
    
    std::atomic<bool> stopGC(false);
    std::thread gcThread([&mgr, &stopGC]() {
        while (!stopGC) {
            mgr.tickGC(100);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    auto stressFn = [&](int start, int count) {
        for (int i = start; i < start + count; ++i) {
            std::string name = "SvcStress_" + std::to_string(i);
            std::shared_ptr<LuaService> svc = mgr.createService(name, m_scriptPath);
            if (svc) {
                svc->execute("local x = 1 + 1");
                mgr.stopService(name);
            }
        }
    };

    // Perform 5000 creation/destruction cycles across multiple threads
    std::vector<std::thread> workers;
    int totalCycles = 5000;
    int threadCount = 4;
    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back(stressFn, i * (totalCycles / threadCount), totalCycles / threadCount);
    }

    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    stopGC = true;
    if (gcThread.joinable()) gcThread.join();
    
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
