# Test Case: `LogicStressTest.LuaInfiniteLoopRecovery`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicStressTest.cpp` (lines 12–35) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LogicStressTest.LuaInfiniteLoopRecovery` defined in `cmake-projects/logicengine/test/LogicStressTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "EvaluationPool" as SUT0
participant "StateMachine" as SUT1
participant "State" as SUT2
participant "Expression" as SUT3
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : EvaluationPool::getInstance().setHardTimeout(std::chrono::millise...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<StateMachine> sm = StateMachine::create( SafetyWa...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : sm->setContextRoot(ctxRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> nominal = State::create( NOMINAL )
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(sm->getCurrentState(), fault)
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `EvaluationPool::getInstance().setHardTimeout(std::chrono::milliseconds(100))`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<StateMachine> sm = StateMachine::create("SafetyWatchdog")`
- `sm->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> nominal = State::create("NOMINAL")`
- `std::shared_ptr<State> fault = State::create("FAULT")`
- `nominal->setInvariant(Expression(lua, "(function() while true do end end)()"))`

## Verification Assertions

- `EXPECT_EQ(sm->getCurrentState(), fault)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
