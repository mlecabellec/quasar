# Test Case: `LuaEngineTest.SandboxExecution`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaEngine.cpp` (lines 48–56) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaEngineTest.SandboxExecution` defined in `cmake-projects/scripting/test/TestLuaEngine.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "TestableLuaEngine" as SUT0
Runner -> SUT0 : auto engine = TestableLuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->doSetupSandbox()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result result = engine->executeString( os...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->shutdown()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(result.valid())
@enduml
```

## Observed Operations

- `auto engine = TestableLuaEngine::create()`
- `engine->doSetupSandbox()`
- `sol::protected_function_result result = engine->executeString("os.execute('echo hi > /dev/null')")`
- `engine->shutdown()`

## Verification Assertions

- `EXPECT_TRUE(result.valid())`

---

[Back to `scripting` Tests Overview](../scripting.md)
