# Test Case: `sim_extended_cs0100_test`

| Property | Value |
| :--- | :--- |
| **Module** | `sim-test` |
| **Source File** | `cmake-projects/sim-test/src/test_sim_extended_cs0100.cpp` (lines 1–151) |
| **Category** | Cross-Module / Third-Party Integration (`CS-0100.16`–`CS-0100.18`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020,`, `CS-0010,`, `CS-0040.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

CS-0100 multi-library SMP simulator integration test inside `sim-test`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "LoadLibrary" as SUT1
participant "CreateInstance" as SUT2
participant "AddModel" as SUT3
Runner -> Logger : TestScopeGuard start (sim_extended_cs0100_test)
loop Bounded test iteration / stress window
Runner -> SUT0 : * brief Constants for multi-model SMP simulation integration test...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Smp::IComponent* invComp = simulator.CreateInstance(invUuid, Inv1...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : Smp::IComponent* simpleComp = simulator.CreateInstance(simpleUuid...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : Smp::IComponent* arrayComp = simulator.CreateInstance(arrayUuid, ...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : simulator.AddModel(invModel)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : simulator.AddModel(simpleModel)
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_NE(invComp, nullptr)
Runner -> Runner : ASSERT_NE(simpleComp, nullptr)
Runner -> Runner : ASSERT_NE(arrayComp, nullptr)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `* @brief Constants for multi-model SMP simulation integration tests (`CS-0040`).`
- `Smp::IComponent* invComp = simulator.CreateInstance(invUuid, "Inv1", "Dynamic Inverter", nullptr)`
- `Smp::IComponent* simpleComp = simulator.CreateInstance(simpleUuid, "DelayScalar1", "Dynamic Scalar", nullptr)`
- `Smp::IComponent* arrayComp = simulator.CreateInstance(arrayUuid, "DelayArray1", "Dynamic Array", nullptr)`
- `simulator.AddModel(invModel)`
- `simulator.AddModel(simpleModel)`
- `simulator.AddModel(arrayModel)`
- `simulator.Publish()`

## Verification Assertions

- `ASSERT_NE(invComp, nullptr)`
- `ASSERT_NE(simpleComp, nullptr)`
- `ASSERT_NE(arrayComp, nullptr)`
- `ASSERT_NE(invModel, nullptr)`
- `ASSERT_NE(simpleModel, nullptr)`
- `ASSERT_NE(arrayModel, nullptr)`
- `ASSERT_NE(invEp, nullptr)`
- `ASSERT_NE(simpleEp, nullptr)`
- `ASSERT_NE(arrayEp, nullptr)`
- `ASSERT_NE(invIn, nullptr)`

---

[Back to `sim-test` Tests Overview](../sim-test.md)
