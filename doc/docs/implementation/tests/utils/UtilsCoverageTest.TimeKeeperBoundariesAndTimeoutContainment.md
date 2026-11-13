# Test Case: `UtilsCoverageTest.TimeKeeperBoundariesAndTimeoutContainment`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 310–331) |
| **Category** | Coverage, Enum, Boundary & Exception Containment (`CS-0100.1`, `CS-0100.3`, `CS-0100.4`, `CS-0100.6`, `CS-0100.9`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.2`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Verifies TimeKeeper boundaries and GetMissionTime nested-lock timeout containment.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SetSimulationTime" as SUT0
participant "SetEpochTime" as SUT1
participant "GetEpochTime" as SUT2
participant "SetMissionStartTime" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsCoverageTest.TimeKeeperBoundarie...)
Runner -> SUT0 : tk.SetSimulationTime(0LL)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : tk.SetEpochTime(-1000LL)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : tk.SetSimulationTime(500LL)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : tk.SetMissionStartTime(100LL)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : quasar::test_support::AsyncTestLogger::instance().logStepDuration...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : UtilsTestConstants::MAX_TEST_BUDGET.count())
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(tk.GetEpochTime(), -1000LL)
Runner -> Runner : EXPECT_EQ(tk.GetEpochTime(), -500LL)
Runner -> Runner : EXPECT_EQ(tk.GetMissionStartTime(), 100LL)
Runner -> Runner : EXPECT_THROW((void)tk.GetMissionTime(), std::runtime_error)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `tk.SetSimulationTime(0LL)`
- `tk.SetEpochTime(-1000LL)`
- `tk.SetSimulationTime(500LL)`
- `tk.SetMissionStartTime(100LL)`
- `quasar::test_support::AsyncTestLogger::instance().logStepDuration("GetMissionTime_timeout", t0, t1)`
- `UtilsTestConstants::MAX_TEST_BUDGET.count())`

## Verification Assertions

- `EXPECT_EQ(tk.GetEpochTime(), -1000LL)`
- `EXPECT_EQ(tk.GetEpochTime(), -500LL)`
- `EXPECT_EQ(tk.GetMissionStartTime(), 100LL)`
- `EXPECT_THROW((void)tk.GetMissionTime(), std::runtime_error)`
- `EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count(),`

---

[Back to `utils` Tests Overview](../utils.md)
