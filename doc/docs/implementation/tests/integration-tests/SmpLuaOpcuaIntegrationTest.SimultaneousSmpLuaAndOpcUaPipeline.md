# Test Case: `SmpLuaOpcuaIntegrationTest.SimultaneousSmpLuaAndOpcUaPipeline`

| Property | Value |
| :--- | :--- |
| **Module** | `integration-tests` |
| **Source File** | `cmake-projects/integration-tests/TestIntegrationSmpLuaOpcua.cpp` (lines 52–105) |
| **Category** | Cross-Module / Third-Party Integration (`CS-0100.16`–`CS-0100.18`) |
| **Traceability** | `[TSK-20260311-005]`, `[FE-0070]`, `[FE-0130]`, `[FE-0200]`, `[CS-0040.2]`, `[CS-0010.37]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Cross-module integration test combining ECSS SMP `Simulator`, `quasar_scripting` Lua, and `quasar_opcua` server/client (`CS-0100.16`, `CS-0100.18`).

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "NamedObject" as SUT0
participant "OpcUaServerService" as SUT1
participant "OpcUaClientService" as SUT2
participant "Publish" as SUT3
Runner -> Logger : TestScopeGuard start (SmpLuaOpcuaIntegrationTest.Simultaneo...)
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<named::NamedObject> plantRoot = named::NamedObjec...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : named::NamedInteger<int64_t>::create( SimTick , 0, plantRoot)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : opcua::OpcUaServerService::create( IntegOpcUaSrv )
SUT2 --> Runner : return / state updated
Runner -> SUT3 : server->setPort(SMP_OPCUA_INTEGRATION_PORT)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : server->setRootObject(plantRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : server->start()
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_NE(client->getChild( SpacecraftBus ), nullptr)
Runner -> Runner : EXPECT_EQ(static_cast<int>(simulator.GetSimulatorState()), static...
Runner -> Runner : ASSERT_TRUE(r.valid())
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<named::NamedObject> plantRoot = named::NamedObject::create("SpacecraftBus")`
- `named::NamedInteger<int64_t>::create("SimTick", 0, plantRoot)`
- `opcua::OpcUaServerService::create("IntegOpcUaSrv")`
- `server->setPort(SMP_OPCUA_INTEGRATION_PORT)`
- `server->setRootObject(plantRoot)`
- `server->start()`
- `(void)waitCv.wait_for(lock, OPCUA_READY_WAIT_MS)`
- `opcua::OpcUaClientService::create("IntegOpcUaCli")`

## Verification Assertions

- `EXPECT_NE(client->getChild("SpacecraftBus"), nullptr)`
- `EXPECT_EQ(static_cast<int>(simulator.GetSimulatorState()), static_cast<int>(Smp::SimulatorStateKind::SSK_Standby))`
- `ASSERT_TRUE(r.valid())`
- `EXPECT_EQ(telemetryTick->value(), 100)`

---

[Back to `integration-tests` Tests Overview](../integration-tests.md)
