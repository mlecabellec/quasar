# Test Case: `SchedulerTutorialTest.Tutorial2_IntermediateCyclicAndImmediate`

| Property | Value |
| :--- | :--- |
| **Module** | `sched` |
| **Source File** | `cmake-projects/sched/test/TestScheduler.cpp` (lines 160–184) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.3`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 2 (Intermediate): Cyclic events, immediate events, and rescheduling.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "AddImmediateEvent" as SUT0
participant "Smp" as SUT1
participant "Services" as SUT2
participant "AddSimulationTimeEvent" as SUT3
Runner -> Logger : TestScopeGuard start (SchedulerTutorialTest.Tutorial2_Inter...)
Runner -> SUT0 : (void)scheduler.AddImmediateEvent( ep_imm)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : scheduler.AddSimulationTimeEvent( ep_cyc, 50LL, SchedTestConstant...
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(scheduler.ExecuteNextEvent(), 0LL)
Runner -> Runner : EXPECT_EQ(imm_hits.load(std::memory_order_relaxed), 1U)
Runner -> Runner : EXPECT_EQ(scheduler.ExecuteNextEvent(), 50LL)
Runner -> Runner : EXPECT_EQ(scheduler.ExecuteNextEvent(), 150LL)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `(void)scheduler.AddImmediateEvent(&ep_imm)`
- `scheduler.AddSimulationTimeEvent(&ep_cyc, 50LL, SchedTestConstants::CYCLE_NS, 2LL)`

## Verification Assertions

- `EXPECT_EQ(scheduler.ExecuteNextEvent(), 0LL)`
- `EXPECT_EQ(imm_hits.load(std::memory_order_relaxed), 1U)`
- `EXPECT_EQ(scheduler.ExecuteNextEvent(), 50LL)`
- `EXPECT_EQ(scheduler.ExecuteNextEvent(), 150LL)`
- `EXPECT_EQ(scheduler.ExecuteNextEvent(), 250LL)`
- `EXPECT_EQ(cyc_hits.load(std::memory_order_relaxed), 3U)`
- `EXPECT_FALSE(scheduler.IsEventScheduled(cyc_id))`

---

[Back to `sched` Tests Overview](../sched.md)
