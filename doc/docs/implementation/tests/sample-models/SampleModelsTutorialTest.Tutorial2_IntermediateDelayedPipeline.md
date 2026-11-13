# Test Case: `SampleModelsTutorialTest.Tutorial2_IntermediateDelayedPipeline`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 250–272) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 2 (Intermediate): Drive `DelayedSimpleModel` through its 3-step pipeline.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Publish" as SUT0
participant "Configure" as SUT1
participant "Connect" as SUT2
participant "Smp" as SUT3
Runner -> Logger : TestScopeGuard start (SampleModelsTutorialTest.Tutorial2_In...)
loop Bounded test iteration / stress window
Runner -> SUT0 : model.Publish( publication)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : model.Configure(nullptr, nullptr)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : model.Connect(nullptr)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : Smp::ISimpleField* inInt = dynamic_cast<Smp::ISimpleField*>(model...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : Smp::ISimpleField* outInt = dynamic_cast<Smp::ISimpleField*>(mode...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : inInt->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32,...
SUT1 --> Runner : return / state updated
end
Runner -> Runner : ASSERT_NE(inInt, nullptr)
Runner -> Runner : ASSERT_NE(outInt, nullptr)
Runner -> Runner : EXPECT_EQ(static_cast<Smp::Int32>(outInt->GetValue()), 42)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `model.Publish(&publication)`
- `model.Configure(nullptr, nullptr)`
- `model.Connect(nullptr)`
- `Smp::ISimpleField* inInt = dynamic_cast<Smp::ISimpleField*>(model.GetField("Input_Int32"))`
- `Smp::ISimpleField* outInt = dynamic_cast<Smp::ISimpleField*>(model.GetField("Output_Int32"))`
- `inInt->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, static_cast<Smp::Int32>(42)))`
- `model.Execute()`
- `quasar::test_support::AsyncTestLogger::instance().logEvent(`

## Verification Assertions

- `ASSERT_NE(inInt, nullptr)`
- `ASSERT_NE(outInt, nullptr)`
- `EXPECT_EQ(static_cast<Smp::Int32>(outInt->GetValue()), 42)`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
