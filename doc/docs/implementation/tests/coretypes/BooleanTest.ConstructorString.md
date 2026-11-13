# Test Case: `BooleanTest.ConstructorString`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBoolean.cpp` (lines 26–59) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BooleanTest.ConstructorString` defined in `cmake-projects/coretypes/test/TestBoolean.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_TRUE(b1.booleanValue())
Runner -> Runner : EXPECT_TRUE(b2.booleanValue())
Runner -> Runner : EXPECT_FALSE(b3.booleanValue())
Runner -> Runner : EXPECT_FALSE(b4.booleanValue())
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_TRUE(b1.booleanValue())`
- `EXPECT_TRUE(b2.booleanValue())`
- `EXPECT_FALSE(b3.booleanValue())`
- `EXPECT_FALSE(b4.booleanValue())`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
