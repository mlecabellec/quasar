# Test Case: `MethodPerformanceTest.CallOverheadComparison`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestMethodPerformance.cpp` (lines 19–75) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Benchmark comparing standard NamedMethod vs TypedNamedMethod call overhead.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedMethod" as SUT0
participant "TypedNamedMethod" as SUT1
participant "ContainerDef" as SUT2
participant "NamedObject" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedMethod> standardMethod = NamedMethod::create...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<TypedNamedMethod> typedMethodNoSchema = TypedName...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<ContainerDef> schema = ContainerDef::create( dumm...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<TypedNamedMethod> typedMethodWithSchema = TypedNa...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> args = NamedObject::create( args )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : standardMethod->execute(args)
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_LT(typedNoSchemaDuration, standardDuration * 2.0)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedMethod> standardMethod = NamedMethod::create("standard", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `std::shared_ptr<TypedNamedMethod> typedMethodNoSchema = TypedNamedMethod::create("typed_no_schema", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `std::shared_ptr<ContainerDef> schema = ContainerDef::create("dummy_schema")`
- `std::shared_ptr<TypedNamedMethod> typedMethodWithSchema = TypedNamedMethod::create("typed_with_schema", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `std::shared_ptr<NamedObject> args = NamedObject::create("args")`
- `standardMethod->execute(args)`
- `int64_t standardDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()`
- `typedMethodNoSchema->execute(args)`

## Verification Assertions

- `EXPECT_LT(typedNoSchemaDuration, standardDuration * 2.0)`

---

[Back to `named` Tests Overview](../named.md)
