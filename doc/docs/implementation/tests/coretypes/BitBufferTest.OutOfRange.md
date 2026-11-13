# Test Case: `BitBufferTest.OutOfRange`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBitBuffer.cpp` (lines 179–202) |
| **Category** | Coverage, Enum, Boundary & Exception Containment (`CS-0100.1`, `CS-0100.3`, `CS-0100.4`, `CS-0100.6`, `CS-0100.9`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BitBufferTest.OutOfRange` defined in `cmake-projects/coretypes/test/TestBitBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_THROW(bb.getBit(8), std::out_of_range)
Runner -> Runner : EXPECT_FALSE(bb.getBit(7))
Runner -> Runner : EXPECT_THROW(bb.setBit(100, true), std::out_of_range)
Runner -> Runner : EXPECT_THROW(bb.sliceBits(5, 5), std::out_of_range) // 5+5=10 > 8
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_THROW(bb.getBit(8), std::out_of_range)`
- `EXPECT_FALSE(bb.getBit(7))`
- `EXPECT_THROW(bb.setBit(100, true), std::out_of_range)`
- `EXPECT_THROW(bb.sliceBits(5, 5), std::out_of_range); // 5+5=10 > 8`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
