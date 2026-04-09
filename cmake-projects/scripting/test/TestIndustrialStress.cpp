#include <gtest/gtest.h>
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

namespace quasar::scripting {

/**
 * @brief Industrial Stress Test: 50 concurrent services updating a shared tree.
 * Fulfills Phase 4: Industrial Validation.
 */
TEST(IndustrialStressTest, HighConcurrencyLockContention) {
    // 1. Setup Host Service and State
    auto hostSvc = LuaService::create("HostSvc");
    sol::state& lua = hostSvc->getEngine()->getState();
    
    // 2. Setup Shared Tree
    auto root = named::NamedObject::create("StressRoot");
    auto globalCounter = named::NamedInteger<int64_t>::create("GlobalCounter", 0, root);
    
    // 3. Define Lua Hook (Increment global counter)
    {
        auto lock = hostSvc->getEngine()->acquireLock();
        lua["globalRoot"] = LuaProxy<named::NamedObject>(root);
        lua.script(R"(
            function stressHook(owner, args)
                if not globalRoot then return end
                local counter = globalRoot:getChild("GlobalCounter")
                if counter and counter:asLong() then
                    local longObj = counter:asLong()
                    local val = longObj:value()
                    longObj:setValue(val + 1)
                end
            end
        )");
    }
    
    sol::function hookFunc;
    {
        auto lock = hostSvc->getEngine()->acquireLock();
        hookFunc = lua["stressHook"];
    }

    // 4. Spawn 50 Services
    const int serviceCount = 50;
    std::vector<std::shared_ptr<named::NamedService>> services;
    
    for (int i = 0; i < serviceCount; ++i) {
        auto svc = named::NamedService::create("Service_" + std::to_string(i), root);
        NamedLuaMethod::create("run", hookFunc, svc);
        svc->setCycleTime(std::chrono::milliseconds(5)); // High frequency
        services.push_back(svc);
    }

    // 5. Start all and run for 2 seconds
    std::cout << "[Stress] Starting 50 concurrent Lua-hooked services..." << std::endl;
    for (auto& svc : services) svc->start();
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "[Stress] Stopping all services..." << std::endl;
    for (auto& svc : services) svc->stop();

    // 6. Verify Results
    int64_t finalCount = globalCounter->value();
    std::cout << "[Stress] Total successful Lua hook executions: " << finalCount << std::endl;
    
    // We expect a significant number of increments.
    EXPECT_GT(finalCount, 500); 
}

} // namespace quasar::scripting
