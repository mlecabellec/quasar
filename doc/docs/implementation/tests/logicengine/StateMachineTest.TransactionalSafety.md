# Test Case: `StateMachineTest.TransactionalSafety`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/StateMachineTest.cpp` (lines 26–71) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `StateMachineTest.TransactionalSafety` defined in `cmake-projects/logicengine/test/StateMachineTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "StateMachine" as SUT0
participant "State" as SUT1
participant "Expression" as SUT2
participant "Transition" as SUT3
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::crea...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<StateMachine> sm = StateMachine::create( CoolingS...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : sm->setContextRoot(ctxRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> idleState = State::create( IDLE )
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(temp->value(), 20)
Runner -> Runner : EXPECT_EQ(sm->getCurrentState(), idleState)
Runner -> Runner : EXPECT_EQ(temp->value(), 32)
Runner -> Runner : EXPECT_NE(sm->getCurrentState(), coolingState)
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<NamedInteger<int>> temp = NamedInteger<int>::create("temperature", 20, ctxRoot)`
- `std::shared_ptr<StateMachine> sm = StateMachine::create("CoolingSystem")`
- `sm->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> idleState = State::create("IDLE")`
- `std::shared_ptr<State> coolingState = State::create("COOLING")`
- `std::shared_ptr<State> alarmState = State::create("ALARM")`

## Verification Assertions

- `EXPECT_EQ(temp->value(), 20)`
- `EXPECT_EQ(sm->getCurrentState(), idleState)`
- `EXPECT_EQ(temp->value(), 32)`
- `EXPECT_NE(sm->getCurrentState(), coolingState)`
- `EXPECT_NE(sm->getCurrentState(), idleState)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
