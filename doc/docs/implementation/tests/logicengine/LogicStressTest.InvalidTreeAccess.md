# Test Case: `LogicStressTest.InvalidTreeAccess`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicStressTest.cpp` (lines 37–59) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LogicStressTest.InvalidTreeAccess` defined in `cmake-projects/logicengine/test/LogicStressTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "StateMachine" as SUT0
participant "State" as SUT1
participant "Expression" as SUT2
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<StateMachine> sm = StateMachine::create( Robustne...
SUT2 --> Runner : return / state updated
Runner -> SUT0 : sm->setContextRoot(ctxRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> s1 = State::create( S1 )
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<State> fault = State::create( FAULT )
SUT2 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(sm->getCurrentState(), fault)
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<StateMachine> sm = StateMachine::create("RobustnessTest")`
- `sm->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> s1 = State::create("S1")`
- `std::shared_ptr<State> fault = State::create("FAULT")`
- `s1->setInvariant(Expression(lua, "1 + nil == 2"))`
- `s1->setFailureState(fault)`

## Verification Assertions

- `EXPECT_EQ(sm->getCurrentState(), fault)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
