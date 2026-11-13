# Test Case: `IntegerTest.CompareTo`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestInteger.cpp` (lines 114–136) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerTest.CompareTo` defined in `cmake-projects/coretypes/test/TestInteger.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Int" as SUT0
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : EXPECT_LT(a.compareTo(b), 0)
Runner -> Runner : EXPECT_GT(b.compareTo(a), 0)
Runner -> Runner : EXPECT_EQ(a.compareTo(a), 0)
Runner -> Runner : EXPECT_TRUE(a.equals(Int(5)))
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_LT(a.compareTo(b), 0)`
- `EXPECT_GT(b.compareTo(a), 0)`
- `EXPECT_EQ(a.compareTo(a), 0)`
- `EXPECT_TRUE(a.equals(Int(5)))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
