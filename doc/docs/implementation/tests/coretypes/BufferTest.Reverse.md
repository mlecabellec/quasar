# Test Case: `BufferTest.Reverse`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 152–162) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.Reverse` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Buffer" as SUT0
Runner -> SUT0 : b.reverse()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(b.toString(), 04030201 )
@enduml
```

## Observed Operations

- `b.reverse()`

## Verification Assertions

- `EXPECT_EQ(b.toString(), "04030201")`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
