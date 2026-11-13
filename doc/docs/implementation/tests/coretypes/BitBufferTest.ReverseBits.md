# Test Case: `BitBufferTest.ReverseBits`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBuffer.cpp` (lines 120–145) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferTest.ReverseBits` defined in `cmake-projects/coretypes/test/TestBitBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : bb.setBit(0, true)
Runner -> Runner : bb.setBit(1, true)
Runner -> Runner : bb.setBit(2, false)
Runner -> Runner : bb.setBit(3, false)
Runner -> Runner : bb.reverseBits()
Runner -> Runner : EXPECT_FALSE(bb.getBit(0))
Runner -> Runner : EXPECT_FALSE(bb.getBit(1))
Runner -> Runner : EXPECT_TRUE(bb.getBit(2))
Runner -> Runner : EXPECT_TRUE(bb.getBit(3))
@enduml
```

## Observed Operations

- `bb.setBit(0, true)`
- `bb.setBit(1, true)`
- `bb.setBit(2, false)`
- `bb.setBit(3, false)`
- `bb.reverseBits()`

## Verification Assertions

- `EXPECT_FALSE(bb.getBit(0))`
- `EXPECT_FALSE(bb.getBit(1))`
- `EXPECT_TRUE(bb.getBit(2))`
- `EXPECT_TRUE(bb.getBit(3))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
