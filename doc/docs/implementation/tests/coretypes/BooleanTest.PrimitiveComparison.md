# Test Case: `BooleanTest.PrimitiveComparison`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBoolean.cpp` (lines 109–139) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BooleanTest.PrimitiveComparison` defined in `cmake-projects/coretypes/test/TestBoolean.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_TRUE(b.equals(true))
Runner -> Runner : EXPECT_FALSE(b.equals(false))
Runner -> Runner : EXPECT_EQ(b.compareTo(true), 0)
Runner -> Runner : EXPECT_GT(b.compareTo(false), 0)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_TRUE(b.equals(true))`
- `EXPECT_FALSE(b.equals(false))`
- `EXPECT_EQ(b.compareTo(true), 0)`
- `EXPECT_GT(b.compareTo(false), 0)`
- `EXPECT_TRUE(b == true)`
- `EXPECT_TRUE(b != false)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
