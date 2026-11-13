# Test Case: `BooleanTest.FromNumeric`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBoolean.cpp` (lines 82–107) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BooleanTest.FromNumeric` defined in `cmake-projects/coretypes/test/TestBoolean.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Boolean" as SUT0
Runner -> SUT0 : Boolean b3 = Boolean::fromNumeric(3.14)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(b1.booleanValue())
Runner -> Runner : EXPECT_FALSE(b2.booleanValue())
Runner -> Runner : EXPECT_TRUE(b3.booleanValue())
@enduml
```

## Observed Operations

- `Boolean b3 = Boolean::fromNumeric(3.14)`

## Verification Assertions

- `EXPECT_TRUE(b1.booleanValue())`
- `EXPECT_FALSE(b2.booleanValue())`
- `EXPECT_TRUE(b3.booleanValue())`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
