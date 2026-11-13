# Test Case: `BufferTest.FromString`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 69–87) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.FromString` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Buffer" as SUT0
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : EXPECT_EQ(b.size(), 4)
Runner -> Runner : EXPECT_EQ(b.get(0), 0xDE)
Runner -> Runner : EXPECT_EQ(b.get(3), 0xEF)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(b.size(), 4)`
- `EXPECT_EQ(b.get(0), 0xDE)`
- `EXPECT_EQ(b.get(3), 0xEF)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
