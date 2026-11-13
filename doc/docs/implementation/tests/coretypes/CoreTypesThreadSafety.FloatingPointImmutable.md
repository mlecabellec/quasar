# Test Case: `CoreTypesThreadSafety.FloatingPointImmutable`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestThreadSafety.cpp` (lines 53–94) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `CoreTypesThreadSafety.FloatingPointImmutable` defined in `cmake-projects/coretypes/test/TestThreadSafety.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
loop Bounded test iteration / stress window
Runner -> Runner : Double sharedDouble(3.14)
Runner -> Runner : threads.emplace_back(reader)
Runner -> Runner : t.join()
end
Runner -> Runner : EXPECT_DOUBLE_EQ(sharedDouble.toDouble(), 3.14)
Runner -> Runner : EXPECT_DOUBLE_EQ(copy.toDouble(), 3.14)
@enduml
```

## Observed Operations

- `Double sharedDouble(3.14)`
- `threads.emplace_back(reader)`
- `t.join()`

## Verification Assertions

- `EXPECT_DOUBLE_EQ(sharedDouble.toDouble(), 3.14)`
- `EXPECT_DOUBLE_EQ(copy.toDouble(), 3.14)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
