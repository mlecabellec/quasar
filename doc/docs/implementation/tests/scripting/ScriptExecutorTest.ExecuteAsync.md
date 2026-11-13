# Test Case: `ScriptExecutorTest.ExecuteAsync`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestScriptExecution.cpp` (lines 34–54) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `ScriptExecutorTest.ExecuteAsync` defined in `cmake-projects/scripting/test/TestScriptExecution.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "LuaService" as SUT1
participant "ScriptExecutor" as SUT2
participant "ExecuteAsync" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : service->start()
SUT2 --> Runner : return / state updated
Runner -> SUT3 : sol::protected_function_result result = service->execute( return ...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : value = result.get<int>()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->stop()
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_EQ(value, 100)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `sol::protected_function_result result = service->execute("return y or 0")`
- `value = result.get<int>()`
- `service->stop()`

## Verification Assertions

- `EXPECT_EQ(value, 100)`

---

[Back to `scripting` Tests Overview](../scripting.md)
