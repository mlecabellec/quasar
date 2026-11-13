# Test Case: `LuaServiceTest.ScriptExecution`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaService.cpp` (lines 45–57) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaServiceTest.ScriptExecution` defined in `cmake-projects/scripting/test/TestLuaService.cpp`.

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
Runner -> SUT0 : sol::protected_function_result result = service->execute( return ...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : int value = result.get<int>()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(result.valid())
Runner -> Runner : EXPECT_EQ(value, 30)
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `sol::protected_function_result result = service->execute("return 10 + 20")`
- `int value = result.get<int>()`
- `service->stop()`

## Verification Assertions

- `ASSERT_TRUE(result.valid())`
- `EXPECT_EQ(value, 30)`

---

[Back to `scripting` Tests Overview](../scripting.md)
