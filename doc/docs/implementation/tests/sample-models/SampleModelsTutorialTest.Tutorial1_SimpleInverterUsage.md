# Test Case: `SampleModelsTutorialTest.Tutorial1_SimpleInverterUsage`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 220–241) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 1 (Basic): Publish and execute `InverterModel` boolean negation.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Publish" as SUT0
participant "GetState" as SUT1
participant "Smp" as SUT2
participant "ComponentStateKind" as SUT3
Runner -> Logger : TestScopeGuard start (SampleModelsTutorialTest.Tutorial1_Si...)
Runner -> SUT0 : inverter.Publish( publication)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : inverter.SetSimpleValue( Input , Smp::AnySimple(Smp::PrimitiveTyp...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : Smp::IObject* execObj = inverter.GetChild( Execute )
SUT2 --> Runner : return / state updated
Runner -> SUT3 : entry->Execute()
SUT3 --> Runner : return / state updated
Runner -> SUT0 : Smp::AnySimple outputVal = inverter.GetSimpleValue( Output )
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(static_cast<int>(inverter.GetState()), static_cast<int>...
Runner -> Runner : ASSERT_NE(execObj, nullptr)
Runner -> Runner : ASSERT_NE(entry, nullptr)
Runner -> Runner : EXPECT_FALSE(static_cast<Smp::Bool>(outputVal))
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `inverter.Publish(&publication)`
- `inverter.SetSimpleValue("Input", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, true))`
- `Smp::IObject* execObj = inverter.GetChild("Execute")`
- `entry->Execute()`
- `Smp::AnySimple outputVal = inverter.GetSimpleValue("Output")`

## Verification Assertions

- `EXPECT_EQ(static_cast<int>(inverter.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Publishing))`
- `ASSERT_NE(execObj, nullptr)`
- `ASSERT_NE(entry, nullptr)`
- `EXPECT_FALSE(static_cast<Smp::Bool>(outputVal))`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
