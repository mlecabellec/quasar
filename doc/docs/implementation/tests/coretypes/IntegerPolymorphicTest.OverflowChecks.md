# Test Case: `IntegerPolymorphicTest.OverflowChecks`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestIntegerPolymorphic.cpp` (lines 207–215) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerPolymorphicTest.OverflowChecks` defined in `cmake-projects/coretypes/test/TestIntegerPolymorphic.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_THROW(max_u64->safeAdd(*one), std::overflow_error)
Runner -> Runner : EXPECT_THROW(min_i64->safeDivide(*neg_one), std::overflow_error)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_THROW(max_u64->safeAdd(*one), std::overflow_error)`
- `EXPECT_THROW(min_i64->safeDivide(*neg_one), std::overflow_error)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
