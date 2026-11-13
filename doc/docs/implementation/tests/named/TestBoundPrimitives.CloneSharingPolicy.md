# Test Case: `TestBoundPrimitives.CloneSharingPolicy`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestBoundPrimitives.cpp` (lines 83–99) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TestBoundPrimitives.CloneSharingPolicy` defined in `cmake-projects/named/test/TestBoundPrimitives.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedBuffer" as SUT0
participant "CopyPolicy" as SUT1
Runner -> SUT0 : std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create( buffer...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedInteger<int64_t>> val = NamedInteger<int64_t...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : val->bind(buffer, 0)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : val->setValue(123) // Now buffer and val are 123
SUT1 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> cloned = val->clone(CopyPolicy::SHARE)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : val->setValue(456)
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(clonedInt->isBound())
Runner -> Runner : ASSERT_EQ(clonedInt->value(), 123)
Runner -> Runner : ASSERT_EQ(clonedInt->value(), 456)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 8)`
- `std::shared_ptr<NamedInteger<int64_t>> val = NamedInteger<int64_t>::create("val", 0)`
- `val->bind(buffer, 0)`
- `val->setValue(123); // Now buffer and val are 123`
- `std::shared_ptr<NamedObject> cloned = val->clone(CopyPolicy::SHARE)`
- `val->setValue(456)`

## Verification Assertions

- `ASSERT_TRUE(clonedInt->isBound())`
- `ASSERT_EQ(clonedInt->value(), 123)`
- `ASSERT_EQ(clonedInt->value(), 456)`

---

[Back to `named` Tests Overview](../named.md)
