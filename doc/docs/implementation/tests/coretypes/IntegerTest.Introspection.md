# Test Case: `IntegerTest.Introspection`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestInteger.cpp` (lines 281–314) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerTest.Introspection` defined in `cmake-projects/coretypes/test/TestInteger.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(i.getType(), Integer )
Runner -> Runner : EXPECT_TRUE(i.isIntegerType())
Runner -> Runner : EXPECT_TRUE(i.isSigned())
Runner -> Runner : EXPECT_EQ(ub.getType(), Integer )
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(i.getType(), "Integer")`
- `EXPECT_TRUE(i.isIntegerType())`
- `EXPECT_TRUE(i.isSigned())`
- `EXPECT_EQ(ub.getType(), "Integer")`
- `EXPECT_TRUE(ub.isIntegerType())`
- `EXPECT_FALSE(ub.isSigned())`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
