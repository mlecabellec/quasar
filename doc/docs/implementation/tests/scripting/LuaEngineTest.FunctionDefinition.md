# Test Case: `LuaEngineTest.FunctionDefinition`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaEngine.cpp` (lines 19–35) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaEngineTest.FunctionDefinition` defined in `cmake-projects/scripting/test/TestLuaEngine.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaEngine" as SUT0
Runner -> SUT0 : auto engine = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->executeString(R (
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function multiply = engine->getState() multiply
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->shutdown()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_NO_THROW(
Runner -> Runner : EXPECT_TRUE(multiply.valid())
Runner -> Runner : EXPECT_EQ(result, 30)
@enduml
```

## Observed Operations

- `auto engine = LuaEngine::create()`
- `engine->executeString(R"(`
- `sol::protected_function multiply = engine->getState()["multiply"]`
- `engine->shutdown()`

## Verification Assertions

- `EXPECT_NO_THROW({`
- `EXPECT_TRUE(multiply.valid())`
- `EXPECT_EQ(result, 30)`

---

[Back to `scripting` Tests Overview](../scripting.md)
