# Test Case: `BufferTest.ComparisonOps`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 281–310) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.ComparisonOps` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Buffer" as SUT0
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : EXPECT_EQ(b1.compareTo(b2), 0)
Runner -> Runner : EXPECT_LT(b1.compareTo(b3), 0)
Runner -> Runner : EXPECT_GT(b3.compareTo(b1), 0)
Runner -> Runner : EXPECT_TRUE(b1.equals(vec))
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(b1.compareTo(b2), 0)`
- `EXPECT_LT(b1.compareTo(b3), 0)`
- `EXPECT_GT(b3.compareTo(b1), 0)`
- `EXPECT_TRUE(b1.equals(vec))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
