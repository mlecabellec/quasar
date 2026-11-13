# Test Case: `NamedTutorialTest.Tutorial1_SimpleNamedPrimitivesAndQuantities`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test_cs0100/TestNamedExtended.cpp` (lines 83–104) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260328-001]`, `[TSK-20260311-001]`, `[FE-0260]`, `[FE-0150]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_named` covering reflexive methods, web methods, traversal rules, CopyPolicy, and temporal/quantity/variant nodes.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "NamedObject" as SUT0
participant "NamedQuantity" as SUT1
participant "Units" as SUT2
participant "NamedDate" as SUT3
Runner -> Logger : TestScopeGuard start (NamedTutorialTest.Tutorial1_SimpleNam...)
Runner -> SUT0 : std::shared_ptr<NamedObject> device = NamedObject::create( PumpSt...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedQuantity::create( FlowRate , 12.5, quasar::coretypes::Units:...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : NamedQuantity::create( RailVoltage , 24.0, V , device)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<NamedDuration> cycle = NamedDuration::create( Cyc...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> clonedFlow = flow->clone(CopyPolicy:...
SUT0 --> Runner : return / state updated
Runner -> Logger : logOutput( flowUnit , flow->getUnitSymbol())
Runner -> Runner : EXPECT_EQ(flow->getType(), NamedQuantity )
Runner -> Runner : EXPECT_EQ(customVolt->getUnitSymbol(), V )
Runner -> Runner : EXPECT_EQ(commissioned->getType(), NamedDate )
Runner -> Runner : EXPECT_EQ(cycle->getType(), NamedDuration )
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> device = NamedObject::create("PumpStation")`
- `NamedQuantity::create("FlowRate", 12.5, quasar::coretypes::Units::Meter, device)`
- `NamedQuantity::create("RailVoltage", 24.0, "V", device)`
- `std::shared_ptr<NamedDuration> cycle = NamedDuration::create("CycleUs", 10000, device)`
- `std::shared_ptr<NamedObject> clonedFlow = flow->clone(CopyPolicy::DUPLICATE)`
- `logOutput("flowUnit", flow->getUnitSymbol())`

## Verification Assertions

- `EXPECT_EQ(flow->getType(), "NamedQuantity")`
- `EXPECT_EQ(customVolt->getUnitSymbol(), "V")`
- `EXPECT_EQ(commissioned->getType(), "NamedDate")`
- `EXPECT_EQ(cycle->getType(), "NamedDuration")`
- `EXPECT_EQ(boot->getType(), "NamedTimestamp")`
- `EXPECT_EQ(clonedFlow->getName(), "FlowRate")`

---

[Back to `named` Tests Overview](../named.md)
