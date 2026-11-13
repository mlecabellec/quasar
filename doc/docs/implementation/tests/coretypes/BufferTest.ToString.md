# Test Case: `BufferTest.ToString`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 56–67) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.ToString` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(b.toString(), deadbeef )
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(b.toString(), "deadbeef")`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
