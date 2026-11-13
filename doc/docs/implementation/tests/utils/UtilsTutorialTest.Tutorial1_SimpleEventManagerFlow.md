# Test Case: `UtilsTutorialTest.Tutorial1_SimpleEventManagerFlow`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 87–108) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.4`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 1 (Simple): Minimal EventManager subscription and emission workflow.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "QueryEventId" as SUT2
participant "Subscribe" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsTutorialTest.Tutorial1_SimpleEve...)
Runner -> SUT0 : const Smp::Services::EventId evt_id = manager.QueryEventId( Tutor...
SUT0 --> Runner : return / state updated
Runner -> Logger : logInput( event_name , TutorialEvent )
Runner -> Logger : logOutput( evt_id , evt_id)
Runner -> SUT3 : manager.Subscribe(evt_id, ep)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : manager.Emit(evt_id, true)
SUT0 --> Runner : return / state updated
Runner -> Logger : logOutput( hits , hits.load())
Runner -> Runner : EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)
Runner -> Runner : EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `const Smp::Services::EventId evt_id = manager.QueryEventId("TutorialEvent")`
- `logInput("event_name", "TutorialEvent")`
- `logOutput("evt_id", evt_id)`
- `manager.Subscribe(evt_id, &ep)`
- `manager.Emit(evt_id, true)`
- `logOutput("hits", hits.load())`
- `manager.Unsubscribe(evt_id, &ep)`
- `manager.Emit(evt_id, true)`

## Verification Assertions

- `EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)`
- `EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U)`

---

[Back to `utils` Tests Overview](../utils.md)
