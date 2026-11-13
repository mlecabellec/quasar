# Test Case: `IndustrialStressTest.HighConcurrencyLockContention`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestIndustrialStress.cpp` (lines 18–76) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Industrial Stress Test: 50 concurrent services updating a shared tree.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaService" as SUT0
participant "NamedObject" as SUT1
participant "Hook" as SUT2
participant "NamedService" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<LuaService> hostSvc = LuaService::create( HostSvc )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sol::state lua = hostSvc->getEngine()->getState()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<named::NamedObject> root = named::NamedObject::cr...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<named::NamedInteger<int64_t>> globalCounter = nam...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::unique_lock<std::recursive_mutex> lock = hostSvc->getEngine(...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : lua.script(R (
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_GT(finalCount, 500)
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> hostSvc = LuaService::create("HostSvc")`
- `sol::state& lua = hostSvc->getEngine()->getState()`
- `std::shared_ptr<named::NamedObject> root = named::NamedObject::create("StressRoot")`
- `std::shared_ptr<named::NamedInteger<int64_t>> globalCounter = named::NamedInteger<int64_t>::create("GlobalCounter", 0, root)`
- `std::unique_lock<std::recursive_mutex> lock = hostSvc->getEngine()->acquireLock()`
- `lua.script(R"(`
- `std::unique_lock<std::recursive_mutex> lock = hostSvc->getEngine()->acquireLock()`
- `std::shared_ptr<named::NamedService> svc = named::NamedService::create("Service_" + std::to_string(i), root)`

## Verification Assertions

- `EXPECT_GT(finalCount, 500)`

---

[Back to `scripting` Tests Overview](../scripting.md)
