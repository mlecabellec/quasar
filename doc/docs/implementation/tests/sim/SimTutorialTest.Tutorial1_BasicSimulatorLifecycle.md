# Test Case: `SimTutorialTest.Tutorial1_BasicSimulatorLifecycle`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 208–227) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.7`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 1 (Basic): Create a `Simulator`, inspect default services, and cycle states.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "GetSimulatorState" as SUT0
participant "Smp" as SUT1
participant "SimulatorStateKind" as SUT2
participant "GetLogger" as SUT3
Runner -> Logger : TestScopeGuard start (SimTutorialTest.Tutorial1_BasicSimula...)
Runner -> SUT0 : simulator.Publish()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : simulator.Configure()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : simulator.Connect()
SUT2 --> Runner : return / state updated
Runner -> SUT3 : simulator.Exit()
SUT3 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind:...
Runner -> Runner : EXPECT_NE(simulator.GetLogger(), nullptr)
Runner -> Runner : EXPECT_NE(simulator.GetTimeKeeper(), nullptr)
Runner -> Runner : EXPECT_NE(simulator.GetScheduler(), nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `simulator.Publish()`
- `simulator.Configure()`
- `simulator.Connect()`
- `simulator.Exit()`

## Verification Assertions

- `EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Building)`
- `EXPECT_NE(simulator.GetLogger(), nullptr)`
- `EXPECT_NE(simulator.GetTimeKeeper(), nullptr)`
- `EXPECT_NE(simulator.GetScheduler(), nullptr)`
- `EXPECT_NE(simulator.GetEventManager(), nullptr)`
- `EXPECT_NE(simulator.GetResolver(), nullptr)`
- `EXPECT_NE(simulator.GetLinkRegistry(), nullptr)`
- `EXPECT_NE(simulator.GetTypeRegistry(), nullptr)`
- `EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Standby)`
- `EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Exiting)`

---

[Back to `sim` Tests Overview](../sim.md)
