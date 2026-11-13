# Test Case: `SchedulerTutorialTest.Tutorial3_AdvancedMultiDomainAndCancel`

| Property | Value |
| :--- | :--- |
| **Module** | `sched` |
| **Source File** | `cmake-projects/sched/test/TestScheduler.cpp` (lines 193–213) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.3`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Multi-domain scheduling (Mission/Epoch) and cancellation.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "AddMissionTimeEvent" as SUT2
participant "AddEpochTimeEvent" as SUT3
Runner -> Logger : TestScopeGuard start (SchedulerTutorialTest.Tutorial3_Advan...)
Runner -> SUT0 : const Smp::Services::EventId mission_id = scheduler.AddMissionTim...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : const Smp::Services::EventId epoch_id = scheduler.AddEpochTimeEve...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : scheduler.RemoveEvent(epoch_id)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : const Smp::Duration fired_time = scheduler.ExecuteNextEvent()
SUT3 --> Runner : return / state updated
Runner -> Runner : EXPECT_FALSE(scheduler.IsEventScheduled(epoch_id))
Runner -> Runner : EXPECT_TRUE(scheduler.IsEventScheduled(mission_id))
Runner -> Runner : EXPECT_EQ(fired_time, 200LL)
Runner -> Runner : EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `const Smp::Services::EventId mission_id = scheduler.AddMissionTimeEvent(&ep, 700LL, 0LL, 0LL)`
- `const Smp::Services::EventId epoch_id = scheduler.AddEpochTimeEvent(&ep, 1300LL, 0LL, 0LL)`
- `scheduler.RemoveEvent(epoch_id)`
- `const Smp::Duration fired_time = scheduler.ExecuteNextEvent()`

## Verification Assertions

- `EXPECT_FALSE(scheduler.IsEventScheduled(epoch_id))`
- `EXPECT_TRUE(scheduler.IsEventScheduled(mission_id))`
- `EXPECT_EQ(fired_time, 200LL)`
- `EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)`

---

[Back to `sched` Tests Overview](../sched.md)
