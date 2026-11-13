# Test Case: `LuaEngineTest.PanicHandling`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaEngine.cpp` (lines 58–64) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaEngineTest.PanicHandling` defined in `cmake-projects/scripting/test/TestLuaEngine.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaEngine" as SUT0
Runner -> SUT0 : auto engine = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result result = engine->executeString( th...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->shutdown()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_FALSE(result.valid())
@enduml
```

## Observed Operations

- `auto engine = LuaEngine::create()`
- `sol::protected_function_result result = engine->executeString("this is not valid lua code $#%#$")`
- `engine->shutdown()`

## Verification Assertions

- `EXPECT_FALSE(result.valid())`

---

[Back to `scripting` Tests Overview](../scripting.md)
