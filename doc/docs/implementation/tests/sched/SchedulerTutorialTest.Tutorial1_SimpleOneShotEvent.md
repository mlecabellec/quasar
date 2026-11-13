# Test Case: `SchedulerTutorialTest.Tutorial1_SimpleOneShotEvent`

| Property | Value |
| :--- | :--- |
| **Module** | `sched` |
| **Source File** | `cmake-projects/sched/test/TestScheduler.cpp` (lines 133–151) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.3`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 1 (Simple): Schedule and execute a one-shot simulation-time event.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "AddSimulationTimeEvent" as SUT2
participant "HasEvents" as SUT3
Runner -> Logger : TestScopeGuard start (SchedulerTutorialTest.Tutorial1_Simpl...)
Runner -> SUT0 : const Smp::Services::EventId id = scheduler.AddSimulationTimeEven...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : const Smp::Duration executed_at = scheduler.ExecuteNextEvent()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(scheduler.HasEvents())
Runner -> Runner : EXPECT_TRUE(scheduler.IsEventScheduled(id))
Runner -> Runner : EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 200LL)
Runner -> Runner : EXPECT_EQ(executed_at, 200LL)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `const Smp::Services::EventId id = scheduler.AddSimulationTimeEvent(&ep, 200LL, 0LL, 0LL)`
- `const Smp::Duration executed_at = scheduler.ExecuteNextEvent()`

## Verification Assertions

- `EXPECT_TRUE(scheduler.HasEvents())`
- `EXPECT_TRUE(scheduler.IsEventScheduled(id))`
- `EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 200LL)`
- `EXPECT_EQ(executed_at, 200LL)`
- `EXPECT_EQ(count.load(std::memory_order_relaxed), 1U)`
- `EXPECT_FALSE(scheduler.HasEvents())`
- `EXPECT_FALSE(scheduler.IsEventScheduled(id))`

---

[Back to `sched` Tests Overview](../sched.md)
