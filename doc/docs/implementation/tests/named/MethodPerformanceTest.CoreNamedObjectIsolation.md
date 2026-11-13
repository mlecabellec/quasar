# Test Case: `MethodPerformanceTest.CoreNamedObjectIsolation`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestMethodPerformance.cpp` (lines 80–98) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Benchmark to prove that new Typed subclasses don't slow down the core NamedObject.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> obj = NamedObject::create( baseline )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::string name = obj->getName()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : int64_t coreDuration = std::chrono::duration_cast<std::chrono::mi...
SUT0 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_LT(coreDuration, 1000000) // Sanity check: < 1s for 1M calls.
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> obj = NamedObject::create("baseline")`
- `std::string name = obj->getName()`
- `int64_t coreDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()`

## Verification Assertions

- `EXPECT_LT(coreDuration, 1000000); // Sanity check: < 1s for 1M calls.`

---

[Back to `named` Tests Overview](../named.md)
