# Test Case: `ScriptExecutorTest.ExecuteSync`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestScriptExecution.cpp` (lines 20–32) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `ScriptExecutorTest.ExecuteSync` defined in `cmake-projects/scripting/test/TestScriptExecution.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "LuaService" as SUT1
participant "ScriptExecutor" as SUT2
participant "ExecuteSync" as SUT3
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : service->start()
SUT2 --> Runner : return / state updated
Runner -> SUT3 : sol::protected_function_result result = service->execute( return x )
SUT3 --> Runner : return / state updated
Runner -> SUT0 : int value = result.get<int>()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->stop()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(value, 42)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `sol::protected_function_result result = service->execute("return x")`
- `int value = result.get<int>()`
- `service->stop()`

## Verification Assertions

- `EXPECT_EQ(value, 42)`

---

[Back to `scripting` Tests Overview](../scripting.md)
