# Test Case: `SampleModelsStressTest.ParallelModelExecutionStress`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 467–494) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Exercises independent `InverterModel` instances in parallel threads for bounded duration.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SampleModelTestConstants" as SUT0
participant "Publish" as SUT1
participant "SetSimpleValue" as SUT2
participant "Smp" as SUT3
Runner -> Logger : TestScopeGuard start (SampleModelsStressTest.ParallelModelE...)
loop Bounded test iteration / stress window
Runner -> SUT0 : workers.reserve(SampleModelTestConstants::WORKER_THREADS)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : workers.emplace_back( t ()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : inv.Publish( pub)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : inv.SetSimpleValue( Input , Smp::AnySimple(Smp::PrimitiveTypeKind...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : inv.Execute()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : workers t .join()
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_EQ(static_cast<Smp::Bool>(inv.GetSimpleValue( Output )), i...
Runner -> Runner : EXPECT_LT(elapsed.count(), 5000)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `workers.reserve(SampleModelTestConstants::WORKER_THREADS)`
- `workers.emplace_back([t]() {`
- `inv.Publish(&pub)`
- `inv.SetSimpleValue("Input", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, input))`
- `inv.Execute()`
- `workers[t].join()`

## Verification Assertions

- `EXPECT_EQ(static_cast<Smp::Bool>(inv.GetSimpleValue("Output")), !input)`
- `EXPECT_LT(elapsed.count(), 5000)`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
