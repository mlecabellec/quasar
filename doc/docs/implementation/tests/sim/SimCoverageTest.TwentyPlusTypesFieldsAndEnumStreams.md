# Test Case: `SimCoverageTest.TwentyPlusTypesFieldsAndEnumStreams`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/TestSimExtended.cpp` (lines 319–397) |
| **Category** | Template / Polymorphic Breadth (`CS-0100.5`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070.9,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tests 20+ primitive types, field publication overloads, and `TypeRegistry` type builders.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "PrimitiveTypeKind" as SUT1
participant "Publication" as SUT2
participant "GetType" as SUT3
Runner -> Logger : TestScopeGuard start (SimCoverageTest.TwentyPlusTypesFields...)
loop Bounded test iteration / stress window
Runner -> SUT0 : Smp::Publication::IType* t = reg.GetType(kinds idx )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : reg.AddFloatType( F , Float , 1U, 1U, 1U, 1U, 1ULL , -1.0, 1.0, f...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : reg.AddIntegerType( I , Int , 2U, 1U, 1U, 1U, 1ULL , 0LL, 100LL, ...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : reg.AddEnumerationType( E , Enum , 3U, 1U, 1U, 1U, 1ULL )
SUT3 --> Runner : return / state updated
Runner -> SUT0 : reg.AddArrayType( A , Arr , 4U, 1U, 1U, 1U, 1ULL , 2U, 1U, 1U, 1U...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : reg.AddStringType( S , Str , 5U, 1U, 1U, 1U, 1ULL , 64U)
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_NE(t, nullptr)
Runner -> Runner : ASSERT_NE(fType, nullptr)
Runner -> Runner : ASSERT_NE(iType, nullptr)
Runner -> Runner : ASSERT_NE(eType, nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `Smp::Publication::IType* t = reg.GetType(kinds[idx])`
- `reg.AddFloatType("F", "Float", {1U, 1U, 1U, 1U, 1ULL}, -1.0, 1.0, false, true, "rad")`
- `reg.AddIntegerType("I", "Int", {2U, 1U, 1U, 1U, 1ULL}, 0LL, 100LL, "pct")`
- `reg.AddEnumerationType("E", "Enum", {3U, 1U, 1U, 1U, 1ULL})`
- `reg.AddArrayType("A", "Arr", {4U, 1U, 1U, 1U, 1ULL}, {2U, 1U, 1U, 1U, 1ULL}, 4U, 8U, true)`
- `reg.AddStringType("S", "Str", {5U, 1U, 1U, 1U, 1ULL}, 64U)`
- `reg.AddStructureType("St", "Struct", {6U, 1U, 1U, 1U, 1ULL})`
- `reg.AddClassType("C", "Class", {7U, 1U, 1U, 1U, 1ULL}, {6U, 1U, 1U, 1U, 1ULL})`

## Verification Assertions

- `EXPECT_NE(t, nullptr)`
- `ASSERT_NE(fType, nullptr)`
- `ASSERT_NE(iType, nullptr)`
- `ASSERT_NE(eType, nullptr)`
- `ASSERT_NE(aType, nullptr)`
- `ASSERT_NE(sType, nullptr)`
- `ASSERT_NE(stType, nullptr)`
- `ASSERT_NE(cType, nullptr)`
- `EXPECT_EQ(sType->GetMaxLength(), 64U)`
- `EXPECT_EQ(aType->GetSize(), 32U)`

---

[Back to `sim` Tests Overview](../sim.md)
