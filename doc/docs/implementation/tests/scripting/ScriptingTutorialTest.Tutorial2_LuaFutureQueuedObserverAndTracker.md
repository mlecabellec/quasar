# Test Case: `ScriptingTutorialTest.Tutorial2_LuaFutureQueuedObserverAndTracker`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test_cs0100/TestScriptingExtended.cpp` (lines 96–127) |
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
participant "LuaService" as SUT0
participant "ObjectTracker" as SUT1
participant "ScriptExecutor" as SUT2
participant "ExecuteSync" as SUT3
Runner -> Logger : TestScopeGuard start (ScriptingTutorialTest.Tutorial2_LuaFu...)
Runner -> SUT0 : std::shared_ptr<LuaService> svc = LuaService::create( ObserverSvc )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : svc->start()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : svc->getEngine()->getState().set_function( onNodeChanged , seenCo...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : seenCount.fetch_add(1)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : svc, svc->getEngine()->getState() onNodeChanged , 10U)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : named::NamedInteger<int>::create( Counter , 1)
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_FALSE(emptyFut.valid())
Runner -> Runner : EXPECT_FALSE(emptyFut.wait())
Runner -> Runner : EXPECT_FALSE(emptyFut.isReady())
Runner -> Runner : EXPECT_THROW((void)emptyFut.get(), std::runtime_error)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> svc = LuaService::create("ObserverSvc")`
- `svc->start()`
- `svc->getEngine()->getState().set_function("onNodeChanged", [&seenCount]() {`
- `seenCount.fetch_add(1)`
- `svc, svc->getEngine()->getState()["onNodeChanged"], 10U)`
- `named::NamedInteger<int>::create("Counter", 1)`
- `ObjectTracker::getInstance().track(counter)`
- `qObs->notify(counter)`

## Verification Assertions

- `EXPECT_FALSE(emptyFut.valid())`
- `EXPECT_FALSE(emptyFut.wait())`
- `EXPECT_FALSE(emptyFut.isReady())`
- `EXPECT_THROW((void)emptyFut.get(), std::runtime_error)`
- `EXPECT_TRUE(ObjectTracker::getInstance().isAlive(counter))`
- `EXPECT_GE(seenCount.load(), 1)`

---

[Back to `scripting` Tests Overview](../scripting.md)
