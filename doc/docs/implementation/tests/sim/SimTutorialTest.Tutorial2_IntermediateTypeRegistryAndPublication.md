# Test Case: `SimTutorialTest.Tutorial2_IntermediateTypeRegistryAndPublication`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 236–261) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.9,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 2 (Intermediate): Register custom SMP types and publish model fields & operations.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Publication" as SUT1
participant "AddFloatType" as SUT2
participant "GetType" as SUT3
Runner -> Logger : TestScopeGuard start (SimTutorialTest.Tutorial2_Intermediat...)
Runner -> SUT0 : types.AddFloatType( Voltage , Bus voltage in Volts , voltageUuid,...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Smp::IField* field = pub.PublishField( BusVoltage , Main bus volt...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : Smp::Publication::IPublishOperation* op = pub.PublishOperation( R...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : op->PublishParameter( mask , Fault bitmask , types.GetType(Smp::P...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : pub.Unpublish()
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(voltageType, nullptr)
Runner -> Runner : EXPECT_EQ(types.GetType(voltageUuid), voltageType)
Runner -> Runner : ASSERT_NE(field, nullptr)
Runner -> Runner : ASSERT_NE(op, nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `types.AddFloatType("Voltage", "Bus voltage in Volts", voltageUuid, -60.0, 60.0, true, true, "V")`
- `Smp::IField* field = pub.PublishField("BusVoltage", "Main bus voltage", &busVoltage)`
- `Smp::Publication::IPublishOperation* op = pub.PublishOperation("ResetFaults", "Clears fault flags")`
- `op->PublishParameter("mask", "Fault bitmask", types.GetType(Smp::PrimitiveTypeKind::PTK_UInt32)->GetUuid(),`
- `pub.Unpublish()`

## Verification Assertions

- `ASSERT_NE(voltageType, nullptr)`
- `EXPECT_EQ(types.GetType(voltageUuid), voltageType)`
- `ASSERT_NE(field, nullptr)`
- `ASSERT_NE(op, nullptr)`
- `EXPECT_EQ(pub.GetField("BusVoltage"), field)`
- `EXPECT_NE(pub.GetFields(), nullptr)`
- `EXPECT_EQ(pub.GetOperation("ResetFaults"), nullptr)`
- `EXPECT_EQ(pub.GetOperations(), nullptr)`
- `EXPECT_EQ(pub.GetProperties(), nullptr)`

---

[Back to `sim` Tests Overview](../sim.md)
