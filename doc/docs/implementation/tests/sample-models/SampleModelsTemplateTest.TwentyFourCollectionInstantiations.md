# Test Case: `SampleModelsTemplateTest.TwentyFourCollectionInstantiations`

| Property | Value |
| :--- | :--- |
| **Module** | `sample-models` |
| **Source File** | `cmake-projects/sample-models/test/TestSampleModels.cpp` (lines 325–355) |
| **Category** | Template / Polymorphic Breadth (`CS-0100.5`) |
| **Traceability** | `TSK-20261113-004`, `FE-0060`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Covers 24 distinct template instantiations of `sample::SimpleCollection<T>`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
Runner -> Logger : TestScopeGuard start (SampleModelsTemplateTest.TwentyFourCo...)
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : EXPECT_TRUE(fields.empty())
Runner -> Runner : EXPECT_TRUE(components.empty())
Runner -> Runner : EXPECT_TRUE(models.empty())
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_TRUE(fields.empty())`
- `EXPECT_TRUE(components.empty())`
- `EXPECT_TRUE(models.empty())`

---

[Back to `sample-models` Tests Overview](../sample-models.md)
