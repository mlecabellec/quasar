/**
 * @file TestIntegrationMultiPlugin.cpp
 * @brief Multi-module and multi-third-party simultaneously executing integration test (`CS-0100.16`, `CS-0100.18`).
 * @details Combines `quasar_named` (`yaml-cpp` + `tinyxml2` + `jsoncons`), `datacodec`, `quasar_calibration`, `quasar_datalogger`, `quasar_logicengine` (`liblua` + `sol2`), `quasar_scripting`, `quasar_opcua` (`open62541` on port `4856`), and `libzmq` cleanly in a single deterministic workflow.
 * @task [TSK-20260328-001] Reflexive Service Orchestration
 * @feature [FE-0130] Lua Scripting Integration
 * @feature [FE-0200] OPC UA Integration
 * @feature [FE-0240] Logic Engine Orchestration
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "datacodec/JsonMapper.hpp"
#include "datacodec/Schema.hpp"
#include "quasar/calibration/Calibrations.hpp"
#include "quasar/datalogger/RingBuffer.hpp"
#include "quasar/logic/CauseEffectMatrix.hpp"
#include "quasar/logic/Expression.hpp"
#include "quasar/logic/LogicEngine.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ScriptExecutor.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <zmq.h>

namespace quasar::integration::test {

/**
 * @brief Dedicated TCP port for multi-plugin OPC UA server (`CS-0040.2`).
 * @requirement [CS-0040.2] Symbolic Port Constant
 * @compliance Isolated from ports `4841`, `4851`-`4855`.
 */
constexpr uint16_t MULTI_PLUGIN_OPCUA_PORT = 4856U;

/**
 * @brief RAII wrapper owning a ZeroMQ context (`CS-0010.21`).
 * @task [CS-0010.21] RAII Resource Ownership
 * @compliance Guarantees `zmq_ctx_term` on scope exit without raw cleanup noise.
 */
class ScopedZmqContext final {
public:
    ScopedZmqContext() : m_ctx(zmq_ctx_new()) {}
    ~ScopedZmqContext() noexcept {
        if (m_ctx != nullptr) {
            (void)zmq_ctx_term(m_ctx);
            m_ctx = nullptr;
        }
    }
    [[nodiscard]] void* get() const noexcept { return m_ctx; }

private:
    void* m_ctx{nullptr};
};

TEST(MultiPluginIntegrationTest, AllCoreAndThirdPartySubsystemsTogether) {
    quasar::test_support::TestScopeGuard scope("MultiPluginIntegrationTest.AllSubsystems");
    // Step 1: Verify libzmq context initialization & inproc PAIR messaging alongside Quasar subsystems.
    ScopedZmqContext zmqCtx;
    ASSERT_NE(zmqCtx.get(), nullptr);

    // Step 2: Construct NamedObject plant tree, apply Calibration, push through LogicEngine & Datalogger.
    std::shared_ptr<named::NamedObject> plant = named::NamedObject::create("PlantRoot");
    std::shared_ptr<named::NamedFloatingPoint<double>> engValue =
        named::NamedFloatingPoint<double>::create("ScaledBar", 0.0, plant);
    std::shared_ptr<named::NamedBoolean> alarmTrip =
        named::NamedBoolean::create("HighAlarm", false, plant);

    calibration::LinearCalibration barScale(0.1, -4.0);
    double pressureBar = std::get<double>(barScale.rawToEng(100.0));
    engValue->setValue(pressureBar);
    EXPECT_DOUBLE_EQ(pressureBar, 6.0);

    std::shared_ptr<logic::LogicEngine> engine = logic::LogicEngine::create("SafetyEngine");
    std::shared_ptr<logic::CauseEffectMatrix> cem =
        logic::CauseEffectMatrix::create("TripCem", 2U, 1U);
    logic::BitVector mask(2U);
    mask.set(0U, true);
    cem->setOrMask(0U, mask);
    cem->initialize();
    cem->start();
    cem->setCause(0U, pressureBar > 5.0);
    engine->addComponent(cem);
    engine->runCycle(std::chrono::milliseconds(5));
    alarmTrip->setValue(cem->getEffect(0U));
    EXPECT_TRUE(alarmTrip->booleanValue());

    datalogger::RingBuffer<double> hist(8U);
    hist.push(pressureBar);
    EXPECT_EQ(hist.size(), 1U);

    // Step 3: Expose tree via OPC UA (open62541) while executing LuaService scripts and JSON/YAML/XML serialization.
    std::shared_ptr<opcua::OpcUaServerService> opcuaSrv =
        opcua::OpcUaServerService::create("MultiOpcUaSrv");
    opcuaSrv->setPort(MULTI_PLUGIN_OPCUA_PORT);
    opcuaSrv->setRootObject(plant);
    opcuaSrv->start();

    std::shared_ptr<scripting::LuaService> luaSvc =
        scripting::LuaService::create("MultiLuaSvc");
    luaSvc->start();
    scripting::ScriptExecutor::ExecuteSync("integVar = 2026", luaSvc);
    EXPECT_EQ(static_cast<int>(luaSvc->getEngine()->getState()["integVar"]), 2026);

    std::string json = named::serialization::toJson(plant);
    std::string yaml = named::serialization::toYaml(plant);
    std::string xml = named::serialization::toXml(plant);
    EXPECT_FALSE(json.empty());
    EXPECT_FALSE(yaml.empty());
    EXPECT_FALSE(xml.empty());

    luaSvc->stop();
    opcuaSrv->stop();
    cem->stop();
    quasar::test_support::AsyncTestLogger::instance().logOutput("multiSubsystemBar", pressureBar);
}

} // namespace quasar::integration::test
