# Test Case: `LuaServiceTest.BasicLifecycle`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaService.cpp` (lines 25–43) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaServiceTest.BasicLifecycle` defined in `cmake-projects/scripting/test/TestLuaService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaService" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->start()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->postTask( executed ()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_TRUE(service->isRunning())
Runner -> Runner : EXPECT_TRUE(executed)
Runner -> Runner : EXPECT_FALSE(service->isRunning())
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `service->postTask([&executed]() {`
- `service->stop()`

## Verification Assertions

- `EXPECT_TRUE(service->isRunning())`
- `EXPECT_TRUE(executed)`
- `EXPECT_FALSE(service->isRunning())`

---

[Back to `scripting` Tests Overview](../scripting.md)
