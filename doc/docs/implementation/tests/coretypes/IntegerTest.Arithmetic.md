# Test Case: `IntegerTest.Arithmetic`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestInteger.cpp` (lines 8–30) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerTest.Arithmetic` defined in `cmake-projects/coretypes/test/TestInteger.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(a.add(b).toInt(), 30)
Runner -> Runner : EXPECT_EQ(a.subtract(b).toInt(), -10)
Runner -> Runner : EXPECT_EQ(a.multiply(b).toInt(), 200)
Runner -> Runner : EXPECT_EQ(b.divide(a).toInt(), 2)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(a.add(b).toInt(), 30)`
- `EXPECT_EQ(a.subtract(b).toInt(), -10)`
- `EXPECT_EQ(a.multiply(b).toInt(), 200)`
- `EXPECT_EQ(b.divide(a).toInt(), 2)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
