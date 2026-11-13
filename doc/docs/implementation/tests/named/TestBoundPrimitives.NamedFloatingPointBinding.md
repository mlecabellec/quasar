# Test Case: `TestBoundPrimitives.NamedFloatingPointBinding`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestBoundPrimitives.cpp` (lines 37–53) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TestBoundPrimitives.NamedFloatingPointBinding` defined in `cmake-projects/named/test/TestBoundPrimitives.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedBuffer" as SUT0
participant "LittleEndian" as SUT1
participant "Endianness" as SUT2
Runner -> SUT0 : std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create( buffer...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedFloatingPoint<double>> val = NamedFloatingPo...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : val->bind(buffer, 8, quasar::coretypes::Endianness::LittleEndian)
SUT2 --> Runner : return / state updated
Runner -> SUT0 : buffer->write<double>(testVal, 8, quasar::coretypes::Endianness::...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : val->setValue(2.71828)
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(val->isBound())
Runner -> Runner : ASSERT_DOUBLE_EQ(val->value(), testVal)
Runner -> Runner : ASSERT_DOUBLE_EQ(buffer->read<double>(8, quasar::coretypes::Endia...
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 16)`
- `std::shared_ptr<NamedFloatingPoint<double>> val = NamedFloatingPoint<double>::create("val", 0.0)`
- `val->bind(buffer, 8, quasar::coretypes::Endianness::LittleEndian)`
- `buffer->write<double>(testVal, 8, quasar::coretypes::Endianness::LittleEndian)`
- `val->setValue(2.71828)`

## Verification Assertions

- `ASSERT_TRUE(val->isBound())`
- `ASSERT_DOUBLE_EQ(val->value(), testVal)`
- `ASSERT_DOUBLE_EQ(buffer->read<double>(8, quasar::coretypes::Endianness::LittleEndian), 2.71828)`

---

[Back to `named` Tests Overview](../named.md)
