# Test Case: `LogicEngineFuzzStressTest.LargeMatrixFuzzAndConcurrentLogicEngineStress`

| Property | Value |
| :--- | :--- |
| **Module** | `logicengine` |
| **Source File** | `cmake-projects/logicengine/test_cs0100/TestLogicEngineExtended.cpp` (lines 249–311) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
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
participant "CauseEffectMatrix" as SUT0
participant "LogicEngine" as SUT1
Runner -> Logger : TestScopeGuard start (LogicEngineFuzzStressTest.LargeMatrix...)
loop Bounded test iteration / stress window
Runner -> SUT0 : CauseEffectMatrix::create( BigMatrix , STRESS_CAUSE_COUNT, STRESS...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : andMask.set(0U, true)
SUT1 --> Runner : return / state updated
Runner -> SUT0 : andMask.set(199U, true)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : matrix->setAndMask(0U, andMask)
SUT1 --> Runner : return / state updated
Runner -> SUT0 : orMask.set(255U, true)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : matrix->setOrMask(1U, orMask)
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_EQ(matrix->getEffect(0U), (c0 c199))
Runner -> Runner : EXPECT_EQ(matrix->getEffect(1U), c255)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `CauseEffectMatrix::create("BigMatrix", STRESS_CAUSE_COUNT, STRESS_EFFECT_COUNT)`
- `andMask.set(0U, true)`
- `andMask.set(199U, true)`
- `matrix->setAndMask(0U, andMask)`
- `orMask.set(255U, true)`
- `matrix->setOrMask(1U, orMask)`
- `matrix->initialize()`
- `matrix->start()`

## Verification Assertions

- `EXPECT_EQ(matrix->getEffect(0U), (c0 && c199))`
- `EXPECT_EQ(matrix->getEffect(1U), c255)`

---

[Back to `logicengine` Tests Overview](../logicengine.md)
