# Test Case: `LuaServiceTest.ErrorHandling`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaService.cpp` (lines 83–94) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaServiceTest.ErrorHandling` defined in `cmake-projects/scripting/test/TestLuaService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaService" as SUT0
Runner -> SUT0 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->start()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result result = service->execute( error( ...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_FALSE(result.valid())
Runner -> Runner : EXPECT_TRUE(std::string(err.what()).find( test error ) = std::str...
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `sol::protected_function_result result = service->execute("error('test error')")`
- `service->stop()`

## Verification Assertions

- `EXPECT_FALSE(result.valid())`
- `EXPECT_TRUE(std::string(err.what()).find("test error") != std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
