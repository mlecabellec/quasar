# Test Case: `LogicEngineTutorialTest.Tutorial2_IntermediateHierarchicalStateMachineWithInvariant`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test_cs0100/TestLogicEngineExtended.cpp` (lines 122–160) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260311-009]`, `[FE-0240]`, `[FE-0240.1]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_logicengine` covering Expression Lua compilation/evaluation, EvaluationPool timeouts/status enums, StateMachine invariants/failure states, SFC parallel steps, and CauseEffectMatrix bit-vector stress.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "NamedObject" as SUT0
participant "NamedBoolean" as SUT1
participant "StateMachine" as SUT2
participant "State" as SUT3
Runner -> Logger : TestScopeGuard start (LogicEngineTutorialTest.Tutorial2_Int...)
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<quasar::named::NamedObject> ctx = quasar::named::...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : quasar::named::NamedBoolean::create( Start , false, ctx)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : quasar::named::NamedBoolean::create( InterlockOk , true, ctx)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<StateMachine> hsm = StateMachine::create( PumpHSM )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> idle = State::create( Idle )
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(hsm->getCurrentState(), running)
Runner -> Runner : EXPECT_EQ(hsm->getCurrentState(), fault)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<quasar::named::NamedObject> ctx = quasar::named::NamedObject::create("Context")`
- `quasar::named::NamedBoolean::create("Start", false, ctx)`
- `quasar::named::NamedBoolean::create("InterlockOk", true, ctx)`
- `std::shared_ptr<StateMachine> hsm = StateMachine::create("PumpHSM")`
- `std::shared_ptr<State> idle = State::create("Idle")`
- `std::shared_ptr<State> running = State::create("Running")`
- `std::shared_ptr<State> fault = State::create("Fault")`

## Verification Assertions

- `EXPECT_EQ(hsm->getCurrentState(), running)`
- `EXPECT_EQ(hsm->getCurrentState(), fault)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
