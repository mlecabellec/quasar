# Test Case: `BooleanTest.ToString`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBoolean.cpp` (lines 61–80) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BooleanTest.ToString` defined in `cmake-projects/coretypes/test/TestBoolean.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(b.toString(), true )
Runner -> Runner : EXPECT_EQ(b2.toString(), false )
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(b.toString(), "true")`
- `EXPECT_EQ(b2.toString(), "false")`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
