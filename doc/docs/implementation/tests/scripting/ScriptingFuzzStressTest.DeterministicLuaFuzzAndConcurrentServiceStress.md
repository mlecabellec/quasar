# Test Case: `ScriptingFuzzStressTest.DeterministicLuaFuzzAndConcurrentServiceStress`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test_cs0100/TestScriptingExtended.cpp` (lines 170–223) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
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
participant "LuaService" as SUT1
participant "ScriptExecutor" as SUT2
participant "ExecuteAsync" as SUT3
Runner -> Logger : TestScopeGuard start (ScriptingFuzzStressTest.Deterministic...)
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<LuaEngine> eng = LuaEngine::create()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : int lhs = rng.nextInt(-500, 500)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : int rhs = rng.nextInt(-500, 500)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : sol::protected_function_result r = eng->executeString(expr)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<LuaService> svc = LuaService::create( ConcurrentL...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : svc->start()
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_TRUE(r.valid())
Runner -> Runner : EXPECT_EQ(out, lhs + rhs)
Runner -> Runner : EXPECT_EQ(completed.load(), LUA_STRESS_WORKERS * SCRIPTS_PER_WORKER)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaEngine> eng = LuaEngine::create()`
- `int lhs = rng.nextInt(-500, 500)`
- `int rhs = rng.nextInt(-500, 500)`
- `sol::protected_function_result r = eng->executeString(expr)`
- `std::shared_ptr<LuaService> svc = LuaService::create("ConcurrentLuaSvc")`
- `svc->start()`
- `workers.reserve(LUA_STRESS_WORKERS)`
- `workers.emplace_back([&svc, &completed, w]() {`

## Verification Assertions

- `ASSERT_TRUE(r.valid())`
- `EXPECT_EQ(out, lhs + rhs)`
- `EXPECT_EQ(completed.load(), LUA_STRESS_WORKERS * SCRIPTS_PER_WORKER)`

---

[Back to `scripting` Tests Overview](../scripting.md)
