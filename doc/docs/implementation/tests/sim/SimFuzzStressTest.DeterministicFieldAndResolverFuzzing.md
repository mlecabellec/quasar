# Test Case: `SimFuzzStressTest.DeterministicFieldAndResolverFuzzing`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 439–473) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | `TSK-20261113-004`, `FE-0060.11,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Performs deterministic seeded fuzzing on `SimpleField`, `SimpleArrayField`, and `Resolver`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "SimTestConstants" as SUT0
participant "Smp" as SUT1
participant "PublishField" as SUT2
participant "PublishArray" as SUT3
Runner -> Logger : TestScopeGuard start (SimFuzzStressTest.DeterministicFieldA...)
loop Bounded test iteration / stress window
Runner -> SUT0 : pub.PublishField( FuzzScalar , Scalar , backingScalar))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Smp::ISimpleArrayField* af = pub.PublishArray(
SUT1 --> Runner : return / state updated
Runner -> SUT2 : backingArr.data(), Smp::PrimitiveTypeKind::PTK_Int32)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : resolver.SetSimulator( simulator)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : const Smp::Int32 val = static_cast<Smp::Int32>(rng.nextInt(-10000...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : rng.nextInt(0, static_cast<std::int64_t>(SimTestConstants::ARRAY_...
SUT1 --> Runner : return / state updated
end
Runner -> Runner : ASSERT_NE(sf, nullptr)
Runner -> Runner : ASSERT_NE(af, nullptr)
Runner -> Runner : EXPECT_EQ(static_cast<Smp::Int32>(sf->GetValue()), val)
Runner -> Runner : EXPECT_EQ(static_cast<Smp::Int32>(af->GetValue(idx)), val)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `pub.PublishField("FuzzScalar", "Scalar", &backingScalar))`
- `Smp::ISimpleArrayField* af = pub.PublishArray(`
- `backingArr.data(), Smp::PrimitiveTypeKind::PTK_Int32)`
- `resolver.SetSimulator(&simulator)`
- `const Smp::Int32 val = static_cast<Smp::Int32>(rng.nextInt(-10000, 10000))`
- `rng.nextInt(0, static_cast<std::int64_t>(SimTestConstants::ARRAY_LEN) - 1))`
- `sf->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, val))`
- `af->SetValue(idx, Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, val))`

## Verification Assertions

- `ASSERT_NE(sf, nullptr)`
- `ASSERT_NE(af, nullptr)`
- `EXPECT_EQ(static_cast<Smp::Int32>(sf->GetValue()), val)`
- `EXPECT_EQ(static_cast<Smp::Int32>(af->GetValue(idx)), val)`
- `EXPECT_EQ(resolver.ResolveAbsolute("/Unknown/Node"), nullptr)`
- `EXPECT_EQ(resolver.ResolveRelative("Missing", &simulator), nullptr)`

---

[Back to `sim` Tests Overview](../sim.md)
