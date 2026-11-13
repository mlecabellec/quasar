# Test Case: `LogicStressTest.TokenMerge`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicStressTest.cpp` (lines 107–140) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LogicStressTest.TokenMerge` defined in `cmake-projects/logicengine/test/LogicStressTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "SFC" as SUT0
participant "State" as SUT1
participant "Expression" as SUT2
participant "Transition" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : lua.open_libraries(sol::lib::base)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedOb...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<SFC> sfc = SFC::create( MergeSFC )
SUT2 --> Runner : return / state updated
Runner -> SUT3 : sfc->setContextRoot(ctxRoot)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<State> bomb = State::create( BOMB )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : bomb->setInvariant(Expression(lua, true ))
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_EQ(active.size(), 1)
Runner -> Runner : EXPECT_EQ((*active.begin())->getName(), BOMB )
@enduml
```

## Observed Operations

- `lua.open_libraries(sol::lib::base)`
- `std::shared_ptr<NamedMap<NamedObject>> ctxRoot = NamedMap<NamedObject>::create("ctx")`
- `std::shared_ptr<SFC> sfc = SFC::create("MergeSFC")`
- `sfc->setContextRoot(ctxRoot)`
- `std::shared_ptr<State> bomb = State::create("BOMB")`
- `bomb->setInvariant(Expression(lua, "true"))`
- `std::shared_ptr<Transition> split1 = Transition::create("SPLIT1", bomb)`
- `split1->setPreCondition(Expression(lua, "true"))`

## Verification Assertions

- `EXPECT_EQ(active.size(), 1)`
- `EXPECT_EQ((*active.begin())->getName(), "BOMB")`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
