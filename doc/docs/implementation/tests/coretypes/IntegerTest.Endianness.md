# Test Case: `IntegerTest.Endianness`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestInteger.cpp` (lines 177–197) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `IntegerTest.Endianness` defined in `cmake-projects/coretypes/test/TestInteger.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Int swapped = val.swapBytes()
Runner -> Runner : Short sSwapped = s.swapBytes()
Runner -> Runner : EXPECT_EQ(swapped.value(), 0x78563412)
Runner -> Runner : EXPECT_EQ(sSwapped.value(), 0x3412)
@enduml
```

## Observed Operations

- `Int swapped = val.swapBytes()`
- `Short sSwapped = s.swapBytes()`

## Verification Assertions

- `EXPECT_EQ(swapped.value(), 0x78563412)`
- `EXPECT_EQ(sSwapped.value(), 0x3412)`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
