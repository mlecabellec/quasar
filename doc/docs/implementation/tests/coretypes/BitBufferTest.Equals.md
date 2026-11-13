# Test Case: `BitBufferTest.Equals`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBuffer.cpp` (lines 204–236) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferTest.Equals` defined in `cmake-projects/coretypes/test/TestBitBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : bb1.setBit(0, true)
Runner -> Runner : bb1.setBit(15, true)
Runner -> Runner : bb2.setBit(0, true)
Runner -> Runner : bb2.setBit(15, true)
Runner -> Runner : bb2.setBit(1, true)
Runner -> Runner : EXPECT_TRUE(bb1.equals(bb2))
Runner -> Runner : EXPECT_FALSE(bb1.equals(bb2))
Runner -> Runner : EXPECT_FALSE(bb1.equals(bb3))
@enduml
```

## Observed Operations

- `bb1.setBit(0, true)`
- `bb1.setBit(15, true)`
- `bb2.setBit(0, true)`
- `bb2.setBit(15, true)`
- `bb2.setBit(1, true)`

## Verification Assertions

- `EXPECT_TRUE(bb1.equals(bb2))`
- `EXPECT_FALSE(bb1.equals(bb2))`
- `EXPECT_FALSE(bb1.equals(bb3))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
