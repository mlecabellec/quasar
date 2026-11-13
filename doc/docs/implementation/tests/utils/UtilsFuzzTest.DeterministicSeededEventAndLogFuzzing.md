# Test Case: `UtilsFuzzTest.DeterministicSeededEventAndLogFuzzing`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 344–363) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070`, `CS-0010.37,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Deterministic seeded fuzzing of EventManager names and Logger strings.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "UtilsTestConstants" as SUT2
participant "QueryEventId" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsFuzzTest.DeterministicSeededEven...)
loop Bounded test iteration / stress window
Runner -> SUT0 : const std::size_t len = static_cast<std::size_t>(rng.nextInt(1, 32))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : const std::string name = rng.nextAsciiString(len)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : const Smp::Services::EventId id_first = manager.QueryEventId(name...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : const Smp::Services::EventId id_second = manager.QueryEventId(nam...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : observed_ids.push_back(id_first)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : logger.Log(nullptr, name.c_str(), static_cast<Smp::Services::LogM...
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_EQ(id_first, id_second)
Runner -> Runner : EXPECT_EQ(observed_ids.size(), UtilsTestConstants::MAX_FUZZ_SAMPLES)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `const std::size_t len = static_cast<std::size_t>(rng.nextInt(1, 32))`
- `const std::string name = rng.nextAsciiString(len)`
- `const Smp::Services::EventId id_first = manager.QueryEventId(name.c_str())`
- `const Smp::Services::EventId id_second = manager.QueryEventId(name.c_str())`
- `observed_ids.push_back(id_first)`
- `logger.Log(nullptr, name.c_str(), static_cast<Smp::Services::LogMessageKind>(rng.nextInt(-2, 6)))`
- `quasar::test_support::AsyncTestLogger::instance().logLoopProgress(`

## Verification Assertions

- `EXPECT_EQ(id_first, id_second)`
- `EXPECT_EQ(observed_ids.size(), UtilsTestConstants::MAX_FUZZ_SAMPLES)`

---

[Back to `utils` Tests Overview](../utils.md)
