# Test Case: `SampleModelsTutorialTest.Tutorial3_AdvancedCompositeArrayAndLifecycle`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 281–312) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004`, `FE-0020,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Compose parent hierarchy, `DelayedArrayModel`, and lifecycle transitions.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "GetParent" as SUT0
participant "GetState" as SUT1
participant "Smp" as SUT2
participant "ComponentStateKind" as SUT3
Runner -> Logger : TestScopeGuard start (SampleModelsTutorialTest.Tutorial3_Ad...)
loop Bounded test iteration / stress window
Runner -> SUT0 : arrayModel.Publish( publication)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : arrayModel.Configure(nullptr, nullptr)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : arrayModel.Connect(nullptr)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayFie...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : Smp::ISimpleArrayField* outArr = dynamic_cast<Smp::ISimpleArrayFi...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : const Smp::UInt64 arrSize = inArr->GetSize()
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_EQ(arrayModel.GetParent(), root)
Runner -> Runner : EXPECT_EQ(static_cast<int>(arrayModel.GetState()), static_cast<in...
Runner -> Runner : ASSERT_NE(inArr, nullptr)
Runner -> Runner : ASSERT_NE(outArr, nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `arrayModel.Publish(&publication)`
- `arrayModel.Configure(nullptr, nullptr)`
- `arrayModel.Connect(nullptr)`
- `Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayField*>(arrayModel.GetField("InputArray"))`
- `Smp::ISimpleArrayField* outArr = dynamic_cast<Smp::ISimpleArrayField*>(arrayModel.GetField("OutputArray"))`
- `const Smp::UInt64 arrSize = inArr->GetSize()`
- `inArr->SetValue(idx, Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int8, static_cast<Smp::Int8>(idx + 10U)))`
- `arrayModel.Execute()`

## Verification Assertions

- `EXPECT_EQ(arrayModel.GetParent(), &root)`
- `EXPECT_EQ(static_cast<int>(arrayModel.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Created))`
- `ASSERT_NE(inArr, nullptr)`
- `ASSERT_NE(outArr, nullptr)`
- `EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(0U)), 10)`
- `EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(9U)), 19)`
- `EXPECT_EQ(static_cast<int>(arrayModel.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Disconnected))`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
