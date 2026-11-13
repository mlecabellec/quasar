# Test Case: `BitVectorTest.BasicOperations`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicEngineTest.cpp` (lines 8–33) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitVectorTest.BasicOperations` defined in `cmake-projects/logicengine/test/LogicEngineTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : bv.set(1, true)
Runner -> Runner : bv.set(5, true)
Runner -> Runner : bv2.set(1, true)
Runner -> Runner : bv2.set(2, true)
Runner -> Runner : EXPECT_EQ(bv.size(), 10)
Runner -> Runner : EXPECT_TRUE(bv.get(1))
Runner -> Runner : EXPECT_TRUE(bv.get(5))
Runner -> Runner : EXPECT_FALSE(bv.get(0))
@enduml
```

## Observed Operations

- `bv.set(1, true)`
- `bv.set(5, true)`
- `bv2.set(1, true)`
- `bv2.set(2, true)`

## Verification Assertions

- `EXPECT_EQ(bv.size(), 10)`
- `EXPECT_TRUE(bv.get(1))`
- `EXPECT_TRUE(bv.get(5))`
- `EXPECT_FALSE(bv.get(0))`
- `EXPECT_TRUE(bv.any())`
- `EXPECT_TRUE(andResult.get(1))`
- `EXPECT_FALSE(andResult.get(5))`
- `EXPECT_FALSE(andResult.get(2))`
- `EXPECT_TRUE(orResult.get(1))`
- `EXPECT_TRUE(orResult.get(2))`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
