# Test Case: `MultiPluginIntegrationTest.AllCoreAndThirdPartySubsystemsTogether`

| Property | Value |
| :--- | :--- |
| **Module** | `integration-tests` |
| **Source File** | `cmake-projects/integration-tests/TestIntegrationMultiPlugin.cpp` (lines 65–125) |
| **Category** | Cross-Module / Third-Party Integration (`CS-0100.16`–`CS-0100.18`) |
| **Traceability** | `[TSK-20260328-001]`, `[FE-0130]`, `[FE-0200]`, `[FE-0240]`, `[CS-0040.2]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Multi-module and multi-third-party simultaneously executing integration test (`CS-0100.16`, `CS-0100.18`).

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "NamedObject" as SUT0
participant "NamedBoolean" as SUT1
participant "LogicEngine" as SUT2
participant "CauseEffectMatrix" as SUT3
Runner -> Logger : TestScopeGuard start (MultiPluginIntegrationTest.AllCoreAnd...)
Runner -> SUT0 : std::shared_ptr<named::NamedObject> plant = named::NamedObject::c...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : named::NamedFloatingPoint<double>::create( ScaledBar , 0.0, plant)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : named::NamedBoolean::create( HighAlarm , false, plant)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : calibration::LinearCalibration barScale(0.1, -4.0)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : double pressureBar = std::get<double>(barScale.rawToEng(100.0))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : engValue->setValue(pressureBar)
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(zmqCtx.get(), nullptr)
Runner -> Runner : EXPECT_DOUBLE_EQ(pressureBar, 6.0)
Runner -> Runner : EXPECT_TRUE(alarmTrip->booleanValue())
Runner -> Runner : EXPECT_EQ(hist.size(), 1U)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<named::NamedObject> plant = named::NamedObject::create("PlantRoot")`
- `named::NamedFloatingPoint<double>::create("ScaledBar", 0.0, plant)`
- `named::NamedBoolean::create("HighAlarm", false, plant)`
- `calibration::LinearCalibration barScale(0.1, -4.0)`
- `double pressureBar = std::get<double>(barScale.rawToEng(100.0))`
- `engValue->setValue(pressureBar)`
- `std::shared_ptr<logic::LogicEngine> engine = logic::LogicEngine::create("SafetyEngine")`
- `logic::CauseEffectMatrix::create("TripCem", 2U, 1U)`

## Verification Assertions

- `ASSERT_NE(zmqCtx.get(), nullptr)`
- `EXPECT_DOUBLE_EQ(pressureBar, 6.0)`
- `EXPECT_TRUE(alarmTrip->booleanValue())`
- `EXPECT_EQ(hist.size(), 1U)`
- `EXPECT_EQ(static_cast<int>(luaSvc->getEngine()->getState()["integVar"]), 2026)`
- `EXPECT_FALSE(json.empty())`
- `EXPECT_FALSE(yaml.empty())`
- `EXPECT_FALSE(xml.empty())`

---

[Back to `integration-tests` Tests Overview](../integration-tests.md)
