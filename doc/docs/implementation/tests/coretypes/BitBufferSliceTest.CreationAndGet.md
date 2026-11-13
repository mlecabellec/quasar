# Test Case: `BitBufferSliceTest.CreationAndGet`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBufferSlice.cpp` (lines 9–36) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferSliceTest.CreationAndGet` defined in `cmake-projects/coretypes/test/TestBitBufferSlice.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : bb->setBit(0, true)
Runner -> Runner : bb->setBit(15, true)
Runner -> Runner : std::shared_ptr<BitBufferSlice> slice = bb->sliceBitsView(0, 16)
Runner -> Runner : EXPECT_EQ(slice->size(), 16)
Runner -> Runner : EXPECT_TRUE(slice->getBit(0))
Runner -> Runner : EXPECT_TRUE(slice->getBit(15))
Runner -> Runner : EXPECT_FALSE(slice->getBit(1))
@enduml
```

## Observed Operations

- `bb->setBit(0, true)`
- `bb->setBit(15, true)`
- `std::shared_ptr<BitBufferSlice> slice = bb->sliceBitsView(0, 16)`

## Verification Assertions

- `EXPECT_EQ(slice->size(), 16)`
- `EXPECT_TRUE(slice->getBit(0))`
- `EXPECT_TRUE(slice->getBit(15))`
- `EXPECT_FALSE(slice->getBit(1))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
