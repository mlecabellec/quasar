# Test Case: `BitBufferStress.HeavySlice`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBufferStress.cpp` (lines 45–73) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferStress.HeavySlice` defined in `cmake-projects/coretypes/test/TestBitBufferStress.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
loop Bounded test iteration / stress window
Runner -> Runner : bb.setBit(i, true)
Runner -> Runner : BitBuffer sliced = bb.sliceBits(0, bits / 2)
end
Runner -> Runner : EXPECT_EQ(sliced.bitSize(), bits / 2)
Runner -> Runner : EXPECT_TRUE(sliced.getBit(0))
Runner -> Runner : EXPECT_FALSE(sliced.getBit(1))
@enduml
```

## Observed Operations

- `bb.setBit(i, true)`
- `BitBuffer sliced = bb.sliceBits(0, bits / 2)`

## Verification Assertions

- `EXPECT_EQ(sliced.bitSize(), bits / 2)`
- `EXPECT_TRUE(sliced.getBit(0))`
- `EXPECT_FALSE(sliced.getBit(1))`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
