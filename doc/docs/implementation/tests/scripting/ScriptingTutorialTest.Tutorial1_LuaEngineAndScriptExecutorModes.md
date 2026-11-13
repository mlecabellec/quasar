# Test Case: `ScriptingTutorialTest.Tutorial1_LuaEngineAndScriptExecutorModes`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test_cs0100/TestScriptingExtended.cpp` (lines 73–94) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260311-011]`, `[TSK-20260328-001]`, `[FE-0130]`, `[CS-0010.37]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_scripting` covering `ScriptExecutor`, `LuaFuture`, `LuaObserver`, `QueuedObserver`, `ObjectTracker`, `ShellUtilities`, exceptions, fuzzing, and concurrency.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "LuaEngine" as SUT0
participant "ScriptExecutor" as SUT1
participant "ExecuteOnce" as SUT2
participant "LuaService" as SUT3
Runner -> Logger : TestScopeGuard start (ScriptingTutorialTest.Tutorial1_LuaEn...)
Runner -> SUT0 : std::shared_ptr<LuaEngine> engine = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sol::protected_function_result res = engine->executeString( retur...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : engine->gcStep(10)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<LuaEngine> secondEngine = LuaEngine::create()
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<LuaService> service = LuaService::create( ExecSvc )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->start()
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(engine, nullptr)
Runner -> Runner : ASSERT_TRUE(res.valid())
Runner -> Runner : EXPECT_EQ(answer, 42)
Runner -> Runner : EXPECT_GT(secondEngine->getId(), engine->getId())
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaEngine> engine = LuaEngine::create()`
- `sol::protected_function_result res = engine->executeString("return 6 * 7")`
- `engine->gcStep(10)`
- `std::shared_ptr<LuaEngine> secondEngine = LuaEngine::create()`
- `std::shared_ptr<LuaService> service = LuaService::create("ExecSvc")`
- `service->start()`
- `service->stop()`
- `logOutput("luaAnswer", answer)`

## Verification Assertions

- `ASSERT_NE(engine, nullptr)`
- `ASSERT_TRUE(res.valid())`
- `EXPECT_EQ(answer, 42)`
- `EXPECT_GT(secondEngine->getId(), engine->getId())`
- `EXPECT_EQ(static_cast<int>(service->getEngine()->getState()["globalPing"]), 99)`

---

[Back to `scripting` Tests Overview](../scripting.md)
