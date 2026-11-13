# Test Case: `NamedObjectThreadSafety.StressTest`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObjectThreadSafety.cpp` (lines 19–120) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Stress test for concurrent NamedObject operations.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
participant "SUCCEED" as SUT1
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : (void)NamedObject::create( child_ + std::to_string(id), root)
SUT1 --> Runner : return / state updated
Runner -> SUT0 : catch (...)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::list<std::shared_ptr<NamedObject>> children = root->getChild...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : std::uniform_int_distribution<size_t> dist(0, children.size() - 1)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::list<std::shared_ptr<NamedObject>>::iterator it = children.b...
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `(void)NamedObject::create("child_" + std::to_string(id), root)`
- `} catch (...) {`
- `std::list<std::shared_ptr<NamedObject>> children = root->getChildren()`
- `std::uniform_int_distribution<size_t> dist(0, children.size() - 1)`
- `std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin()`
- `(*it)->setParent(nullptr); // Remove`
- `std::list<std::shared_ptr<NamedObject>> children = root->getChildren()`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `named` Tests Overview](../named.md)
