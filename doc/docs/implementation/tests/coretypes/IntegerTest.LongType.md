# Test Case: `IntegerTest.LongType`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestInteger.cpp` (lines 139–158) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerTest.LongType` defined in `cmake-projects/coretypes/test/TestInteger.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(l.add(l2).toLong(), 300L)
Runner -> Runner : EXPECT_THROW(max.safeAdd(one), std::overflow_error)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(l.add(l2).toLong(), 300L)`
- `EXPECT_THROW(max.safeAdd(one), std::overflow_error)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
