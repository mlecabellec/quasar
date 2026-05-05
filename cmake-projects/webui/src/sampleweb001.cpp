/**
 * @file sampleweb001.cpp
 * @brief Integration sample demonstrating NamedObject tree, OPC UA, LogicEngine, and WebUI.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-008.6] Sample Web Integration.
 * - Adheres to [CS-0010] and [CS-0020] standards.
 */

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/logic/LogicEngine.hpp"
#include "quasar/logic/StateMachine.hpp"
#include "quasar/logic/State.hpp"
#include "quasar/logic/Transition.hpp"
#include "quasar/logic/Expression.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/webui/WebUIService.hpp"
#include "quasar/webui/CmrcResourceProvider.hpp"
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(quasar_web);
#include <iostream>
#include <chrono>
#include <thread>
#include <sol/sol.hpp>

using namespace quasar::named;
using namespace quasar::logic;
using namespace quasar::opcua;
using namespace quasar::webui;

/**
 * @brief Main entry point for the sample.
 */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    try {
        std::cout << "[Sample] Initializing Quasar Integration Reference..." << std::endl;

        // --- 1. Construct the NamedObject Hierarchy ---
        std::shared_ptr<NamedObject> universe = NamedObject::create("Universe");
        std::shared_ptr<NamedObject> registry = NamedObject::create("Registry", universe);
        
        // Sensors Subtree
        std::shared_ptr<NamedObject> sensors = NamedObject::create("Sensors", registry);
        std::shared_ptr<NamedFloatingPoint<double>> temp = NamedFloatingPoint<double>::create("Temperature", 21.5, sensors);
        std::shared_ptr<NamedInteger<int64_t>> press = NamedInteger<int64_t>::create("Pressure", 1013, sensors);

        // Status & Command Subtree
        std::shared_ptr<NamedObject> status = NamedObject::create("Status", registry);
        std::shared_ptr<NamedBoolean> isRunning = NamedBoolean::create("isRunning", false, status);
        std::shared_ptr<NamedString> sysState = NamedString::create("systemState", "STANDBY", status);
        std::shared_ptr<NamedBoolean> cmdStart = NamedBoolean::create("cmdStart", false, status);
        std::shared_ptr<NamedBoolean> cmdStop = NamedBoolean::create("cmdStop", false, status);

        // --- 2. Setup Logic Engine & State Machine ---
        std::shared_ptr<LogicEngine> engine = LogicEngine::create("Logic", universe);
        std::shared_ptr<StateMachine> fsm = StateMachine::create("SystemFSM", engine);
        fsm->setContextRoot(registry);

        // Define States
        std::shared_ptr<State> standby = State::create("STANDBY", fsm);
        std::shared_ptr<State> starting = State::create("STARTING", fsm);
        std::shared_ptr<State> started = State::create("STARTED", fsm);
        std::shared_ptr<State> stopping = State::create("STOPPING", fsm);

        // Setup Lua for Expression Compilation
        sol::state lua;
        lua.open_libraries(sol::lib::base);

        // Define Transitions (Option A: Direct Variable Binding)
        // STANDBY -> STARTING when cmdStart is true
        std::shared_ptr<Transition> t1 = Transition::create("StartTrigger", starting, standby);
        t1->setPreCondition(Expression(lua, "ctx.Status.cmdStart == true"));

        // STARTING -> STARTED (Immediate for sample, or after some delay logic)
        std::shared_ptr<Transition> t2 = Transition::create("StartComplete", started, starting);
        t2->setPreCondition(Expression(lua, "true")); // Immediate

        // STARTED -> STOPPING when cmdStop is true
        std::shared_ptr<Transition> t3 = Transition::create("StopTrigger", stopping, started);
        t3->setPreCondition(Expression(lua, "ctx.Status.cmdStop == true"));

        // STOPPING -> STANDBY (Immediate)
        std::shared_ptr<Transition> t4 = Transition::create("StopComplete", standby, stopping);
        t4->setPreCondition(Expression(lua, "true"));

        // Attach state update logic via Actions
        struct StateAction : public IAction {
            std::shared_ptr<NamedString> m_label;
            std::string m_value;
            std::shared_ptr<NamedBoolean> m_cmdStart;
            std::shared_ptr<NamedBoolean> m_cmdStop;
            
            StateAction(std::shared_ptr<NamedString> label, std::string value, 
                        std::shared_ptr<NamedBoolean> cmdStart = nullptr, 
                        std::shared_ptr<NamedBoolean> cmdStop = nullptr) 
                : m_label(label), m_value(value), m_cmdStart(cmdStart), m_cmdStop(cmdStop) {}
            
            void execute(std::shared_ptr<NamedObject> ctx = nullptr) override { 
                (void)ctx; 
                m_label->setValue(m_value); 
                // [CS-0010.44] Reset command flags once consumed.
                if (m_cmdStart) m_cmdStart->setValue(false);
                if (m_cmdStop) m_cmdStop->setValue(false);
            }
        };

        standby->setOnEntry(std::make_shared<StateAction>(sysState, "STANDBY", nullptr, cmdStop));
        starting->setOnEntry(std::make_shared<StateAction>(sysState, "STARTING", cmdStart, nullptr));
        started->setOnEntry(std::make_shared<StateAction>(sysState, "STARTED"));
        stopping->setOnEntry(std::make_shared<StateAction>(sysState, "STOPPING", nullptr, cmdStop));

        fsm->setInitialState(standby);
        engine->addComponent(fsm);

        // --- 3. Deploy Industrial Services ---
        
        // OPC UA Server
        std::shared_ptr<OpcUaServerService> opcua = OpcUaServerService::create("OPCUA", universe);
        opcua->setPort(4840);
        opcua->setRootObject(registry);
        // OpcUaServerService does not have a public initialize(), it handles it in start().

        // Web UI Service
        std::shared_ptr<WebUIService> webui = WebUIService::create("WebUI", 8086, universe);
        webui->addResourceProvider(std::make_unique<CmrcResourceProvider>(cmrc::quasar_web::get_filesystem()));
        webui->setWebRoot("./cmake-projects/webui/frontend/dist");
        // WebUIService has a public initialize() from its manual definition.
        webui->initialize();

        // --- 4. Main Execution Loop ---
        std::cout << "[Sample] Starting services..." << std::endl;
        opcua->start();
        webui->start();
        fsm->initialize();
        fsm->start();
        std::cout << "[Sample] FSM State after start: " << (int)fsm->getState() << std::endl;

        std::cout << "[Sample] Mission Control active at http://localhost:8086" << std::endl;
        std::cout << "[Sample] OPC UA Endpoint at opc.tcp://localhost:4840" << std::endl;

        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point lastCycle = startTime;
        
        // Capped run: 3600 seconds (1 hour)
        while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(3600)) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            std::chrono::steady_clock::duration dt = now - lastCycle;
            lastCycle = now;

            // Drive Logic Engine
            engine->runCycle(std::chrono::duration_cast<std::chrono::nanoseconds>(dt));

            // Simulation: Oscillate Temperature
            double tVal = 20.0 + 5.0 * std::sin(std::chrono::duration<double>(now - startTime).count() * 0.5);
            temp->setValue(tVal);

            // Update isRunning based on FSM
            isRunning->setValue(fsm->getCurrentState()->getName() == "STARTED");

            if (cmdStart->booleanValue() || cmdStop->booleanValue()) {
                std::cout << "[Sample] Current State: " << fsm->getCurrentState()->getName() 
                          << " cmdStart: " << cmdStart->booleanValue() 
                          << " cmdStop: " << cmdStop->booleanValue() << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "[Sample] Stopping services..." << std::endl;
        fsm->stop();
        webui->stop();
        opcua->stop();

    } catch (const std::exception& e) {
        std::cerr << "[Sample] CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[Sample] Execution finished." << std::endl;
    return 0;
}
