# Test Case: `BitBufferTest.ReverseBitsGroup`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBuffer.cpp` (lines 147–177) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferTest.ReverseBitsGroup` defined in `cmake-projects/coretypes/test/TestBitBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : bb.setBit(0, true)
Runner -> Runner : bb.setBit(1, false) // 10
Runner -> Runner : bb.setBit(2, true)
Runner -> Runner : bb.setBit(3, true) // 11
Runner -> Runner : bb.setBit(4, false)
Runner -> Runner : bb.setBit(5, false) // 00
Runner -> Runner : EXPECT_FALSE(bb.getBit(0))
Runner -> Runner : EXPECT_FALSE(bb.getBit(1))
Runner -> Runner : EXPECT_TRUE(bb.getBit(2))
Runner -> Runner : EXPECT_TRUE(bb.getBit(3))
@enduml
```

## Observed Operations

- `bb.setBit(0, true)`
- `bb.setBit(1, false); // 10`
- `bb.setBit(2, true)`
- `bb.setBit(3, true); // 11`
- `bb.setBit(4, false)`
- `bb.setBit(5, false); // 00`
- `bb.reverseBits(2)`

## Verification Assertions

- `EXPECT_FALSE(bb.getBit(0))`
- `EXPECT_FALSE(bb.getBit(1))`
- `EXPECT_TRUE(bb.getBit(2))`
- `EXPECT_TRUE(bb.getBit(3))`
- `EXPECT_TRUE(bb.getBit(4))`
- `EXPECT_FALSE(bb.getBit(5))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
