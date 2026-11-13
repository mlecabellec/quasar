# Test Case: `ScriptingTutorialTest.Tutorial3_ShellUtilitiesAndPluginBoundaries`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test_cs0100/TestScriptingExtended.cpp` (lines 129–140) |
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
participant "ShellUtilities" as SUT0
participant "LuaEngine" as SUT1
Runner -> Logger : TestScopeGuard start (ScriptingTutorialTest.Tutorial3_Shell...)
Runner -> SUT0 : std::shared_ptr<LuaEngine> eng = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : bindShellUtilities(eng->getState())
SUT1 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result helpRes = eng->executeString( help...
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(helpRes.valid())
Runner -> Runner : EXPECT_FALSE(loader.loadPlugin( /nonexistent/missing_plugin.so , ...
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaEngine> eng = LuaEngine::create()`
- `bindShellUtilities(eng->getState())`
- `sol::protected_function_result helpRes = eng->executeString("help()")`

## Verification Assertions

- `EXPECT_TRUE(helpRes.valid())`
- `EXPECT_FALSE(loader.loadPlugin("/nonexistent/missing_plugin.so", eng->getState()))`

---

[Back to `scripting` Tests Overview](../scripting.md)
