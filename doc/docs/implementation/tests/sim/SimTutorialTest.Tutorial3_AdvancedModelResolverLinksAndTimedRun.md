# Test Case: `SimTutorialTest.Tutorial3_AdvancedModelResolverLinksAndTimedRun`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 270–306) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.5,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Wire `ProbeModel`, `Resolver`, `LinkRegistry`, and timed `Run(duration)`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "AddModel" as SUT0
participant "AddInitEntryPoint" as SUT1
participant "Publish" as SUT2
participant "Configure" as SUT3
Runner -> Logger : TestScopeGuard start (SimTutorialTest.Tutorial3_AdvancedMod...)
Runner -> SUT0 : ProbeModel* sensor = sensorOwned.get()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : ProbeModel* sink = sinkOwned.get()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : simulator.AddModel(sensorOwned.release())
SUT2 --> Runner : return / state updated
Runner -> SUT3 : simulator.AddModel(sinkOwned.release())
SUT3 --> Runner : return / state updated
Runner -> SUT0 : simulator.AddInitEntryPoint( initEp)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : simulator.Publish()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind:...
Runner -> Runner : EXPECT_EQ(initEp.GetExecutions(), 1U)
Runner -> Runner : ASSERT_NE(resolver, nullptr)
Runner -> Runner : EXPECT_EQ(resolvedSensor, sensor)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `ProbeModel* sensor = sensorOwned.get()`
- `ProbeModel* sink = sinkOwned.get()`
- `simulator.AddModel(sensorOwned.release())`
- `simulator.AddModel(sinkOwned.release())`
- `simulator.AddInitEntryPoint(&initEp)`
- `simulator.Publish()`
- `simulator.Configure()`
- `simulator.Connect()`

## Verification Assertions

- `EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Standby)`
- `EXPECT_EQ(initEp.GetExecutions(), 1U)`
- `ASSERT_NE(resolver, nullptr)`
- `EXPECT_EQ(resolvedSensor, sensor)`
- `ASSERT_NE(links, nullptr)`
- `EXPECT_EQ(links->GetLinkCount(sensor, sink), 1U)`
- `EXPECT_FALSE(links->CanRemove(sink))`
- `EXPECT_TRUE(links->CanRemove(sink))`

---

[Back to `sim` Tests Overview](../sim.md)
