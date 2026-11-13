# Test Case: `SimFuzzStressTest.ConcurrentLinkRegistryStress`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 482–513) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.6`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Exercises concurrent `LinkRegistry` link/query/remove cycles across threads.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SimTestConstants" as SUT0
participant "AddLink" as SUT1
participant "Smp" as SUT2
participant "GetLinkCount" as SUT3
Runner -> Logger : TestScopeGuard start (SimFuzzStressTest.ConcurrentLinkRegis...)
loop Bounded test iteration / stress window
Runner -> SUT0 : workers.reserve(SimTestConstants::CONCURRENCY_THREADS)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : workers.emplace_back( registry, models, target, t ()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : registry.AddLink( models t , target)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : const Smp::UInt32 count = registry.GetLinkCount( models t , target)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : workers t .join()
SUT0 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_GE(count, 1U)
Runner -> Runner : EXPECT_TRUE(registry.RemoveLink( models t , target))
Runner -> Runner : EXPECT_LT(elapsed.count(), 5000)
Runner -> Runner : EXPECT_TRUE(registry.CanRemove( target))
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `workers.reserve(SimTestConstants::CONCURRENCY_THREADS)`
- `workers.emplace_back([&registry, &models, &target, t]() {`
- `registry.AddLink(&models[t], &target)`
- `const Smp::UInt32 count = registry.GetLinkCount(&models[t], &target)`
- `workers[t].join()`

## Verification Assertions

- `EXPECT_GE(count, 1U)`
- `EXPECT_TRUE(registry.RemoveLink(&models[t], &target))`
- `EXPECT_LT(elapsed.count(), 5000)`
- `EXPECT_TRUE(registry.CanRemove(&target))`

---

[Back to `sim` Tests Overview](../sim.md)
