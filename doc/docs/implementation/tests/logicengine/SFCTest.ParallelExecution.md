# Test Case: `SFCTest.ParallelExecution`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/SFCTest.cpp` (lines 9–53) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `SFCTest.ParallelExecution` defined in `cmake-projects/logicengine/test/SFCTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "SFC" as SUT0
participant "State" as SUT1
participant "Expression" as SUT2
participant "Transition" as SUT3
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<NamedInteger<int>> trigger = NamedInteger<int>::c...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<SFC> sfc = SFC::create( ParallelSFC )
SUT3 --> Runner : return / state updated
Runner -> SUT0 : sfc->setContextRoot(ctxRoot)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<State> step1 = State::create( STEP1 )
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(active.size(), 2)
Runner -> Runner : EXPECT_TRUE(active.contains(step2a))
Runner -> Runner : EXPECT_TRUE(active.contains(step2b))
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<NamedInteger<int>> trigger = NamedInteger<int>::create("trigger", 0, ctxRoot)`
- `std::shared_ptr<SFC> sfc = SFC::create("ParallelSFC")`
- `sfc->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> step1 = State::create("STEP1")`
- `std::shared_ptr<State> step2a = State::create("STEP2A")`
- `std::shared_ptr<State> step2b = State::create("STEP2B")`

## Verification Assertions

- `EXPECT_EQ(active.size(), 2)`
- `EXPECT_TRUE(active.contains(step2a))`
- `EXPECT_TRUE(active.contains(step2b))`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
