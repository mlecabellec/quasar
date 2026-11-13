# Test Case: `LogicEngineTest.Orchestration`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test/LogicEngineTest.cpp` (lines 74–89) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LogicEngineTest.Orchestration` defined in `cmake-projects/logicengine/test/LogicEngineTest.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LogicEngine" as SUT0
participant "CauseEffectMatrix" as SUT1
Runner -> SUT0 : std::shared_ptr<LogicEngine> engine = LogicEngine::create( Engine )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<CauseEffectMatrix> matrix = CauseEffectMatrix::cr...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : matrix->initialize()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : matrix->start()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : engine->addComponent(matrix)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : andMask.set(0, true)
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(matrix->getEffect(0))
@enduml
```

## Observed Operations

- `std::shared_ptr<LogicEngine> engine = LogicEngine::create("Engine")`
- `std::shared_ptr<CauseEffectMatrix> matrix = CauseEffectMatrix::create("Matrix", 2, 1)`
- `matrix->initialize()`
- `matrix->start()`
- `engine->addComponent(matrix)`
- `andMask.set(0, true)`
- `matrix->setAndMask(0, andMask)`
- `matrix->setCause(0, true)`

## Verification Assertions

- `EXPECT_TRUE(matrix->getEffect(0))`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
