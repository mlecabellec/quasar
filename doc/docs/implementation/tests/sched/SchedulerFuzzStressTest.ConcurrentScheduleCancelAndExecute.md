# Test Case: `SchedulerFuzzStressTest.ConcurrentScheduleCancelAndExecute`

| Property | Value |
| :--- | :--- |
| **Module** | `sched` |
| **Source File** | `cmake-projects/sched/test/TestScheduler.cpp` (lines 362–409) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.3`, `CS-0010.37,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Concurrent producers scheduling/cancelling while consumer drains timeline safely.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SchedTestConstants" as SUT0
participant "Smp" as SUT1
participant "Services" as SUT2
participant "AddSimulationTimeEvent" as SUT3
Runner -> Logger : TestScopeGuard start (SchedulerFuzzStressTest.ConcurrentSch...)
loop Bounded test iteration / stress window
Runner -> SUT0 : scheduler.AddSimulationTimeEvent( ep, static_cast<Smp::Duration>(...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : (void)scheduler.IsEventScheduled(id)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : scheduler.RemoveEvent(id)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : (void)scheduler.ExecuteNextEvent()
SUT3 --> Runner : return / state updated
Runner -> SUT0 : producer.join()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : consumer.join()
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_FALSE(scheduler.HasEvents())
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `scheduler.AddSimulationTimeEvent(&ep, static_cast<Smp::Duration>(idx + 1U), 0LL, 0LL)`
- `(void)scheduler.IsEventScheduled(id)`
- `scheduler.RemoveEvent(id)`
- `(void)scheduler.ExecuteNextEvent()`
- `producer.join()`
- `consumer.join()`
- `(void)scheduler.ExecuteNextEvent()`

## Verification Assertions

- `EXPECT_FALSE(scheduler.HasEvents())`

---

[Back to `sched` Tests Overview](../sched.md)
