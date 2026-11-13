# Test Case: `DataloggerTutorialTest.Tutorial1_SimpleRingBufferAndDataView`

| Property | Value |
| :--- | :--- |
| **Module** | `datalogger` |
| **Source File** | `cmake-projects/datalogger/test_cs0100/TestDataloggerExtended.cpp` (lines 98–115) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004,`, `FE-0160`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 1 (Basic): Push/pop samples in `RingBuffer` and sample a `NamedObject` via `DataView`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "DataloggerTestConstants" as SUT0
Runner -> Logger : TestScopeGuard start (DataloggerTutorialTest.Tutorial1_Simp...)
Runner -> SUT0 : const std::optional<int> first = rb.pop()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : quasar::named::NamedInteger<int64_t>::create( RPM , 1500)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : const std::optional<quasar::datalogger::DataSample> sample = view...
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(rb.push(10))
Runner -> Runner : EXPECT_TRUE(rb.push(20))
Runner -> Runner : ASSERT_TRUE(first.has_value())
Runner -> Runner : EXPECT_EQ(first.value(), 10)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `const std::optional<int> first = rb.pop()`
- `quasar::named::NamedInteger<int64_t>::create("RPM", 1500)`
- `const std::optional<quasar::datalogger::DataSample> sample = view.sample()`

## Verification Assertions

- `EXPECT_TRUE(rb.push(10))`
- `EXPECT_TRUE(rb.push(20))`
- `ASSERT_TRUE(first.has_value())`
- `EXPECT_EQ(first.value(), 10)`
- `ASSERT_TRUE(sample.has_value())`
- `EXPECT_EQ(sample.value().sourcePath, "RPM")`
- `EXPECT_TRUE(std::holds_alternative<double>(sample.value().value))`

---

[Back to `datalogger` Tests Overview](../datalogger.md)
