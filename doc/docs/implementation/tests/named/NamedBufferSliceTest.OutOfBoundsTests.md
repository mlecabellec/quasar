# Test Case: `NamedBufferSliceTest.OutOfBoundsTests`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedSlices.cpp` (lines 146–167) |
| **Category** | Coverage, Enum, Boundary & Exception Containment (`CS-0100.1`, `CS-0100.3`, `CS-0100.4`, `CS-0100.6`, `CS-0100.9`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedBufferSliceTest.OutOfBoundsTests` defined in `cmake-projects/named/test/TestNamedSlices.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedBuffer" as SUT0
participant "NamedBufferSlice" as SUT1
Runner -> SUT0 : std::shared_ptr<NamedBuffer> buf = NamedBuffer::create( buf , 10)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedBufferSlice::create( slice2 , buf, 2, 5)
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_THROW(NamedBufferSlice::create( slice1 , buf, 5, 6),
Runner -> Runner : EXPECT_THROW(NamedBufferSlice::create( slice1 , buf, 15, 1),
Runner -> Runner : EXPECT_THROW(NamedBufferSlice::create( subSlice1 , validSlice, 3,...
Runner -> Runner : EXPECT_THROW(validSlice->sliceView(3, 3), std::out_of_range)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("buf", 10)`
- `NamedBufferSlice::create("slice2", buf, 2, 5)`

## Verification Assertions

- `EXPECT_THROW(NamedBufferSlice::create("slice1", buf, 5, 6),`
- `EXPECT_THROW(NamedBufferSlice::create("slice1", buf, 15, 1),`
- `EXPECT_THROW(NamedBufferSlice::create("subSlice1", validSlice, 3, 3),`
- `EXPECT_THROW(validSlice->sliceView(3, 3), std::out_of_range)`

---

[Back to `named` Tests Overview](../named.md)
