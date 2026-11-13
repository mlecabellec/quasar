# Test Case: `UtilsTutorialTest.Tutorial3_AdvancedMultiServiceComposition`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 153–173) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.1,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Multi-service Logger, TimeKeeper, and re-entrant EventManager chain.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "Services" as SUT1
participant "QueryLogMessageKind" as SUT2
participant "ILogger" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsTutorialTest.Tutorial3_AdvancedM...)
Runner -> SUT0 : logger.QueryLogMessageKind(Smp::Services::ILogger::LMK_Informatio...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : tk.SetEventManager( manager)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : tk.SetSimulationTime(2500LL)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : tk.SetMissionTime(500LL)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : const Smp::DateTime zulu_now = tk.GetZuluTime()
SUT0 --> Runner : return / state updated
Runner -> Logger : logOutput( zulu_now , zulu_now)
Runner -> Runner : EXPECT_EQ(info_kind, Smp::Services::ILogger::LMK_Information)
Runner -> Runner : EXPECT_GT(zulu_now, 0LL)
Runner -> Runner : EXPECT_EQ(logger.GetState(), Smp::ComponentStateKind::CSK_Connected)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `logger.QueryLogMessageKind(Smp::Services::ILogger::LMK_InformationName)`
- `tk.SetEventManager(&manager)`
- `tk.SetSimulationTime(2500LL)`
- `tk.SetMissionTime(500LL)`
- `const Smp::DateTime zulu_now = tk.GetZuluTime()`
- `logOutput("zulu_now", zulu_now)`
- `logger.Log(&tk, "Tutorial 3 simulation checkpoint", info_kind)`

## Verification Assertions

- `EXPECT_EQ(info_kind, Smp::Services::ILogger::LMK_Information)`
- `EXPECT_GT(zulu_now, 0LL)`
- `EXPECT_EQ(logger.GetState(), Smp::ComponentStateKind::CSK_Connected)`

---

[Back to `utils` Tests Overview](../utils.md)
