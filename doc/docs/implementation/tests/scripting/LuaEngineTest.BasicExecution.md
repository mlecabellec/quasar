# Test Case: `LuaEngineTest.BasicExecution`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaEngine.cpp` (lines 7–17) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaEngineTest.BasicExecution` defined in `cmake-projects/scripting/test/TestLuaEngine.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaEngine" as SUT0
Runner -> SUT0 : auto engine = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->executeString( x = 10 ny = 20 nresult = x + y )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : int result = engine->getState() result
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->shutdown()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_NO_THROW(
Runner -> Runner : EXPECT_EQ(result, 30)
@enduml
```

## Observed Operations

- `auto engine = LuaEngine::create()`
- `engine->executeString("x = 10\ny = 20\nresult = x + y")`
- `int result = engine->getState()["result"]`
- `engine->shutdown()`

## Verification Assertions

- `EXPECT_NO_THROW({`
- `EXPECT_EQ(result, 30)`

---

[Back to `scripting` Tests Overview](../scripting.md)
