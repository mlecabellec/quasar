# Test Case: `BufferTest.ConstructAndSize`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 10–31) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.ConstructAndSize` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(b1.size(), 0)
Runner -> Runner : EXPECT_EQ(b2.size(), 10)
Runner -> Runner : EXPECT_EQ(b2.get(0), 0)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(b1.size(), 0)`
- `EXPECT_EQ(b2.size(), 10)`
- `EXPECT_EQ(b2.get(0), 0)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
