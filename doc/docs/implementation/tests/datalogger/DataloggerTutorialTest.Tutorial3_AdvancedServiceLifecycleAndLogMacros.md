# Test Case: `DataloggerTutorialTest.Tutorial3_AdvancedServiceLifecycleAndLogMacros`

| Property | Value |
| :--- | :--- |
| **Module** | `datalogger` |
| **Source File** | `cmake-projects/datalogger/test_cs0100/TestDataloggerExtended.cpp` (lines 164–185) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004,`, `FE-0160`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Configure singleton `DataLoggerService` via `initDefault` and invoke `LOG_*` macros.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "DataLoggerService" as SUT0
participant "LOG_DEBUG" as SUT1
participant "LOG_INFO" as SUT2
participant "LOG_WARN" as SUT3
Runner -> Logger : TestScopeGuard start (DataloggerTutorialTest.Tutorial3_Adva...)
Runner -> SUT0 : svc->flush()
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(svc, nullptr)
Runner -> Runner : EXPECT_EQ(svc->getType(), DataLoggerService )
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `svc->flush()`

## Verification Assertions

- `ASSERT_NE(svc, nullptr)`
- `EXPECT_EQ(svc->getType(), "DataLoggerService")`

---

[Back to `datalogger` Tests Overview](../datalogger.md)
