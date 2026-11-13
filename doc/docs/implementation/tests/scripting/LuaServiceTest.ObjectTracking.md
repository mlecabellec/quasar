# Test Case: `LuaServiceTest.ObjectTracking`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaService.cpp` (lines 59–81) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaServiceTest.ObjectTracking` defined in `cmake-projects/scripting/test/TestLuaService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaService" as SUT0
participant "ObjectTracker" as SUT1
Runner -> SUT0 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->start()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result result = service->execute(R (
SUT0 --> Runner : return / state updated
Runner -> SUT1 : local obj = quasar.named.createObject( Standalone )
SUT1 --> Runner : return / state updated
Runner -> SUT0 : sol::object obj = result.get<sol::object>()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->stop()
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(result.valid())
Runner -> Runner : ASSERT_NE(standalone, nullptr)
Runner -> Runner : EXPECT_EQ(standalone->getName(), Standalone )
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `sol::protected_function_result result = service->execute(R"(`
- `local obj = quasar.named.createObject("Standalone")`
- `sol::object obj = result.get<sol::object>()`
- `service->stop()`
- `standalone.reset()`

## Verification Assertions

- `ASSERT_TRUE(result.valid())`
- `ASSERT_NE(standalone, nullptr)`
- `EXPECT_EQ(standalone->getName(), "Standalone")`

---

[Back to `scripting` Tests Overview](../scripting.md)
