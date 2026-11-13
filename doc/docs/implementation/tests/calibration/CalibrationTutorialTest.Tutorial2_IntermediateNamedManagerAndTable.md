# Test Case: `CalibrationTutorialTest.Tutorial2_IntermediateNamedManagerAndTable`

| Property | Value |
| :--- | :--- |
| **Module** | `calibration` |
| **Source File** | `cmake-projects/calibration/test/TestCalibrationExtended.cpp` (lines 86–131) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004,`, `FE-0050,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 2 (Intermediate): Organize named calibrations in `CalibrationManager` + piecewise table.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "AttachableManager" as SUT0
participant "CalibrationManager" as SUT1
participant "AttachableLinear" as SUT2
participant "NamedLinearCalibration" as SUT3
Runner -> Logger : TestScopeGuard start (CalibrationTutorialTest.Tutorial2_Int...)
Runner -> SUT0 : /** brief Local bridge exposing addChild / setSelf on Calibration...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : /** brief Local bridge calling setSelf on NamedLinearCalibration ...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : ptr->setSelf(ptr)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : /** brief Local bridge calling setSelf on NamedPolynomialCalibrat...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : ptr->setSelf(ptr)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<AttachableLinear> lin = AttachableLinear::create(...
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(mgr->getType(), CalibrationManager )
Runner -> Runner : EXPECT_EQ(mgr->getCalibration( BusScale ), lin)
Runner -> Runner : EXPECT_EQ(mgr->getCalibration( MissingCal ), nullptr)
Runner -> Runner : EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(5.0...
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `/** @brief Local bridge exposing `addChild`/`setSelf` on `CalibrationManager` for attachment (`CS-0090.1`). */`
- `/** @brief Local bridge calling `setSelf` on `NamedLinearCalibration` so `as<NamedCalibration>()` works (`CS-0090.1`). */`
- `ptr->setSelf(ptr)`
- `/** @brief Local bridge calling `setSelf` on `NamedPolynomialCalibration` so `as<NamedCalibration>()` works (`CS-0090.1`). */`
- `ptr->setSelf(ptr)`
- `std::shared_ptr<AttachableLinear> lin = AttachableLinear::create("BusScale", 2.0, 1.5)`
- `AttachablePoly::create("Thermistor", std::vector<double>{1.0, 2.0, 0.5})`
- `mgr->attach(lin)`

## Verification Assertions

- `EXPECT_EQ(mgr->getType(), "CalibrationManager")`
- `EXPECT_EQ(mgr->getCalibration("BusScale"), lin)`
- `EXPECT_EQ(mgr->getCalibration("MissingCal"), nullptr)`
- `EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(5.0)), 50.0)`
- `EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(15.0)), 200.0)`
- `EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(-5.0)), 0.0)`
- `EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(25.0)), 300.0)`

---

[Back to `calibration` Tests Overview](../calibration.md)
