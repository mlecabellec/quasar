# Test Case: `UtilsTutorialTest.Tutorial2_IntermediateTimeKeeperWithEvents`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 117–144) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.2,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 2 (Intermediate): Coupled TimeKeeper and EventManager notifications.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "QueryEventId" as SUT2
participant "IEventManager" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsTutorialTest.Tutorial2_Intermedi...)
Runner -> SUT0 : manager.QueryEventId(Smp::Services::IEventManager::SMP_EpochTimeC...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : manager.QueryEventId(Smp::Services::IEventManager::SMP_MissionTim...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : manager.Subscribe(epoch_id, ep_epoch)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : manager.Subscribe(mission_id, ep_mission)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : tk.SetEventManager( manager)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : tk.SetSimulationTime(1000LL)
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(tk.GetSimulationTime(), 1000LL)
Runner -> Runner : EXPECT_EQ(tk.GetEpochTime(), 5000LL)
Runner -> Runner : EXPECT_EQ(tk.GetMissionStartTime(), 2000LL)
Runner -> Runner : EXPECT_EQ(epoch_hits.load(std::memory_order_relaxed), 1U)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `manager.QueryEventId(Smp::Services::IEventManager::SMP_EpochTimeChanged)`
- `manager.QueryEventId(Smp::Services::IEventManager::SMP_MissionTimeChanged)`
- `manager.Subscribe(epoch_id, &ep_epoch)`
- `manager.Subscribe(mission_id, &ep_mission)`
- `tk.SetEventManager(&manager)`
- `tk.SetSimulationTime(1000LL)`
- `tk.SetEpochTime(5000LL)`
- `tk.SetMissionStartTime(2000LL)`

## Verification Assertions

- `EXPECT_EQ(tk.GetSimulationTime(), 1000LL)`
- `EXPECT_EQ(tk.GetEpochTime(), 5000LL)`
- `EXPECT_EQ(tk.GetMissionStartTime(), 2000LL)`
- `EXPECT_EQ(epoch_hits.load(std::memory_order_relaxed), 1U)`
- `EXPECT_EQ(mission_hits.load(std::memory_order_relaxed), 1U)`

---

[Back to `utils` Tests Overview](../utils.md)
