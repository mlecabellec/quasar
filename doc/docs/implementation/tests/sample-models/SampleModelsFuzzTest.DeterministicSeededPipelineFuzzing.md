# Test Case: `SampleModelsFuzzTest.DeterministicSeededPipelineFuzzing`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 405–458) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Performs deterministic seeded fuzzing across `DelayedSimpleModel`, `DelayedArrayModel`, and `InverterModel`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SampleModelTestConstants" as SUT0
participant "Publish" as SUT1
participant "Smp" as SUT2
participant "GetField" as SUT3
Runner -> Logger : TestScopeGuard start (SampleModelsFuzzTest.DeterministicSee...)
loop Bounded test iteration / stress window
Runner -> SUT0 : inverter.Publish( publication)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : scalar.Publish( publication)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : arr.Publish( publication)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : Smp::ISimpleField* inF64 = dynamic_cast<Smp::ISimpleField*>(scala...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : Smp::ISimpleField* outF64 = dynamic_cast<Smp::ISimpleField*>(scal...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayFie...
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_NE(inF64, nullptr)
Runner -> Runner : ASSERT_NE(outF64, nullptr)
Runner -> Runner : ASSERT_NE(inArr, nullptr)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `inverter.Publish(&publication)`
- `scalar.Publish(&publication)`
- `arr.Publish(&publication)`
- `Smp::ISimpleField* inF64 = dynamic_cast<Smp::ISimpleField*>(scalar.GetField("Input_Float64"))`
- `Smp::ISimpleField* outF64 = dynamic_cast<Smp::ISimpleField*>(scalar.GetField("Output_Float64"))`
- `Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayField*>(arr.GetField("InputArray"))`
- `Smp::ISimpleArrayField* outArr = dynamic_cast<Smp::ISimpleArrayField*>(arr.GetField("OutputArray"))`
- `const bool bit = rng.nextInt(0, 1) != 0`

## Verification Assertions

- `ASSERT_NE(inF64, nullptr)`
- `ASSERT_NE(outF64, nullptr)`
- `ASSERT_NE(inArr, nullptr)`
- `ASSERT_NE(outArr, nullptr)`
- `EXPECT_EQ(static_cast<Smp::Bool>(inverter.GetSimpleValue("Output")), !bit)`
- `EXPECT_DOUBLE_EQ(static_cast<Smp::Float64>(outF64->GetValue()), hist2)`
- `EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(0U)), arrHist2)`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
