# Test Case: `NamedBufferSliceTest.Clone`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedSlices.cpp` (lines 115–144) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedBufferSliceTest.Clone` defined in `cmake-projects/named/test/TestNamedSlices.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedBufferSlice" as SUT0
Runner -> SUT0 : buf->set(5, 0xFF)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : NamedBufferSlice::create( s , buf, 5, 1)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> copy = slice->clone()
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(casted, nullptr)
Runner -> Runner : EXPECT_EQ(casted->get(0), 0xFF)
@enduml
```

## Observed Operations

- `buf->set(5, 0xFF)`
- `NamedBufferSlice::create("s", buf, 5, 1)`
- `std::shared_ptr<NamedObject> copy = slice->clone()`

## Verification Assertions

- `ASSERT_NE(casted, nullptr)`
- `EXPECT_EQ(casted->get(0), 0xFF)`

---

[Back to `named` Tests Overview](../named.md)
