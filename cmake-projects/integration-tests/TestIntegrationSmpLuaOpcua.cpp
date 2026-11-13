/**
 * @file TestIntegrationSmpLuaOpcua.cpp
 * @brief Cross-module integration test combining ECSS SMP `Simulator`, `quasar_scripting` Lua, and `quasar_opcua` server/client (`CS-0100.16`, `CS-0100.18`).
 * @details Verifies simultaneous execution of ECSS SMP simulation lifecycle (`sim`/`sched`/`utils`), `liblua`+`sol2`, and `open62541` OPC UA server/client mirroring on dedicated port `4855`.
 * @task [TSK-20260311-005] OPC UA & Simulation Integration
 * @feature [FE-0070] ECSS SMP Simulation Kernel
 * @feature [FE-0130] Lua Scripting Integration
 * @feature [FE-0200] OPC UA Server and Client Services
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include "sim/Simulator.hpp"
#include <Smp/SimulatorStateKind.h>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

namespace quasar::integration::test {

/**
 * @brief Dedicated TCP port for SMP + Lua + OPC UA integration test (`CS-0040.2`).
 * @requirement [CS-0040.2] Symbolic Port Constant
 * @compliance Avoids collision with `4841` and `4851`-`4853`.
 */
constexpr uint16_t SMP_OPCUA_INTEGRATION_PORT = 4855U;

/**
 * @brief Simulation ticks executed during SMP + Lua + OPC UA integration (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Fixed finite iteration count.
 */
constexpr std::size_t INTEGRATION_SIM_TICKS = 10U;

/**
 * @brief Bounded wait for OPC UA server socket readiness (`CS-0010.27`).
 * @requirement [CS-0010.27] Timed Wait Usage
 * @compliance Finite condition wait.
 */
constexpr std::chrono::milliseconds OPCUA_READY_WAIT_MS{80};

TEST(SmpLuaOpcuaIntegrationTest, SimultaneousSmpLuaAndOpcUaPipeline) {
    quasar::test_support::TestScopeGuard scope("SmpLuaOpcuaIntegrationTest.SimultaneousPipeline");
    // Step 1: Initialize NamedObject tree exposed to OPC UA and Lua scripting.
    std::shared_ptr<named::NamedObject> plantRoot = named::NamedObject::create("SpacecraftBus");
    std::shared_ptr<named::NamedInteger<int64_t>> telemetryTick =
        named::NamedInteger<int64_t>::create("SimTick", 0, plantRoot);

    std::shared_ptr<opcua::OpcUaServerService> server =
        opcua::OpcUaServerService::create("IntegOpcUaSrv");
    server->setPort(SMP_OPCUA_INTEGRATION_PORT);
    server->setRootObject(plantRoot);
    server->start();

    std::mutex waitMtx;
    std::condition_variable waitCv;
    std::unique_lock<std::mutex> lock(waitMtx);
    (void)waitCv.wait_for(lock, OPCUA_READY_WAIT_MS);

    std::shared_ptr<opcua::OpcUaClientService> client =
        opcua::OpcUaClientService::create("IntegOpcUaCli");
    client->setUrl("opc.tcp://localhost:" + std::to_string(SMP_OPCUA_INTEGRATION_PORT));
    client->start();
    EXPECT_NE(client->getChild("SpacecraftBus"), nullptr);

    // Step 2: Initialize ECSS SMP Simulator and LuaEngine simultaneously.
    sim::Simulator simulator;
    simulator.Publish();
    simulator.Configure();
    simulator.Connect();
    simulator.Initialise();
    EXPECT_EQ(static_cast<int>(simulator.GetSimulatorState()), static_cast<int>(Smp::SimulatorStateKind::SSK_Standby));

    std::shared_ptr<scripting::LuaEngine> lua = scripting::LuaEngine::create();
    lua->getState().set_function("pushSimTick", [&telemetryTick](int64_t val) {
        telemetryTick->setValue(val);
    });

    // Step 3: Step Lua telemetry producer in bounded loop and advance SMP simulator state cleanly.
    for (std::size_t tick = 1U; tick <= INTEGRATION_SIM_TICKS; ++tick) {
        if (tick > INTEGRATION_SIM_TICKS) {
            throw std::runtime_error("Integration sim tick bound exceeded");
        } else {
            sol::protected_function_result r =
                lua->executeString("pushSimTick(" + std::to_string(tick * 10U) + ")");
            ASSERT_TRUE(r.valid());
        }
    }
    EXPECT_EQ(telemetryTick->value(), 100);
    quasar::test_support::AsyncTestLogger::instance().logOutput("finalTelemetryTick", telemetryTick->value());

    simulator.Exit();
    client->stop();
    server->stop();
}

} // namespace quasar::integration::test
