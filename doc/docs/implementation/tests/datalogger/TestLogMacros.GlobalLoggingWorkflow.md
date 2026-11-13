# Test Case: `TestLogMacros.GlobalLoggingWorkflow`

| Property | Value |
| :--- | :--- |
| **Module** | `datalogger` |
| **Source File** | `cmake-projects/datalogger/test/TestLogMacros.cpp` (lines 37–76) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TestLogMacros.GlobalLoggingWorkflow` defined in `cmake-projects/datalogger/test/TestLogMacros.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LOG_INFO" as SUT0
participant "LOG_WARN" as SUT1
participant "LOG_ERROR" as SUT2
participant "DataLoggerService" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : inst->flush()
SUT0 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_TRUE(file.is_open())
Runner -> Runner : EXPECT_EQ(matchesFound, 3)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `inst->flush()`

## Verification Assertions

- `ASSERT_TRUE(file.is_open())`
- `EXPECT_EQ(matchesFound, 3)`

---

[Back to `datalogger` Tests Overview](../datalogger.md)
