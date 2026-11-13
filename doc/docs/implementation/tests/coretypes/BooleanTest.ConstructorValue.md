# Test Case: `BooleanTest.ConstructorValue`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBoolean.cpp` (lines 7–24) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BooleanTest.ConstructorValue` defined in `cmake-projects/coretypes/test/TestBoolean.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_TRUE(b.booleanValue())
Runner -> Runner : EXPECT_FALSE(b2.booleanValue())
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_TRUE(b.booleanValue())`
- `EXPECT_FALSE(b2.booleanValue())`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
