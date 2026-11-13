# Test Case: `MethodPerformanceTest.ValidationRejection`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestMethodPerformance.cpp` (lines 103–116) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verify that TypedNamedMethod rejects missing arguments when a schema is present.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "ContainerDef" as SUT0
participant "TypedNamedMethod" as SUT1
participant "NamedObject" as SUT2
Runner -> SUT0 : std::shared_ptr<ContainerDef> schema = ContainerDef::create( inpu...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<TypedNamedMethod> method = TypedNamedMethod::crea...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<NamedObject> args = NamedObject::create( args )
SUT2 --> Runner : return / state updated
Runner -> Runner : EXPECT_THROW(method->execute(nullptr), std::invalid_argument)
Runner -> Runner : EXPECT_NO_THROW(method->execute(args))
@enduml
```

## Observed Operations

- `std::shared_ptr<ContainerDef> schema = ContainerDef::create("input_schema")`
- `std::shared_ptr<TypedNamedMethod> method = TypedNamedMethod::create("test", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `std::shared_ptr<NamedObject> args = NamedObject::create("args")`

## Verification Assertions

- `EXPECT_THROW(method->execute(nullptr), std::invalid_argument)`
- `EXPECT_NO_THROW(method->execute(args))`

---

[Back to `named` Tests Overview](../named.md)
