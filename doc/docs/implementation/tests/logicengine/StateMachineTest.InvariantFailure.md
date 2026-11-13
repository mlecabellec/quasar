# Test Case: `StateMachineTest.InvariantFailure`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/StateMachineTest.cpp` (lines 73–102) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `StateMachineTest.InvariantFailure` defined in `cmake-projects/logicengine/test/StateMachineTest.cpp`.

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
Runner -> SUT2 : std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::crea...
SUT2 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<StateMachine> sm = StateMachine::create( SafetySM )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sm->setContextRoot(ctxRoot)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<State> safeState = State::create( SAFE )
SUT2 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(sm->getCurrentState(), safeState)
Runner -> Runner : EXPECT_EQ(sm->getCurrentState(), faultState)
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::create("temperature", 20, ctxRoot)`
- `std::shared_ptr<StateMachine> sm = StateMachine::create("SafetySM")`
- `sm->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> safeState = State::create("SAFE")`
- `std::shared_ptr<State> faultState = State::create("FAULT")`
- `safeState->setInvariant(Expression(lua, "ctx.temperature < 100"))`

## Verification Assertions

- `EXPECT_EQ(sm->getCurrentState(), safeState)`
- `EXPECT_EQ(sm->getCurrentState(), faultState)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
