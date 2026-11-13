# Test Case: `LogicStressTest.ParallelDeterminism`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicStressTest.cpp` (lines 74–105) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LogicStressTest.ParallelDeterminism` defined in `cmake-projects/logicengine/test/LogicStressTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "SFC" as SUT0
participant "State" as SUT1
participant "Transition" as SUT2
participant "Expression" as SUT3
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<NamedInteger<int>> counter = NamedInteger<int>::c...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<SFC> sfc = SFC::create( DeterministicSFC )
SUT3 --> Runner : return / state updated
Runner -> SUT0 : sfc->setContextRoot(ctxRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> start = State::create( START )
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(counter->value(), 2)
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<NamedInteger<int>> counter = NamedInteger<int>::create("counter", 0, ctxRoot)`
- `std::shared_ptr<SFC> sfc = SFC::create("DeterministicSFC")`
- `sfc->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> start = State::create("START")`
- `std::shared_ptr<State> endA = State::create("ENDA")`
- `std::shared_ptr<State> endB = State::create("ENDB")`

## Verification Assertions

- `EXPECT_EQ(counter->value(), 2)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
