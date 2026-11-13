# Test Case: `NamedObjectStress.DeepHierarchy`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObjectStress.cpp` (lines 11–39) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectStress.DeepHierarchy` defined in `cmake-projects/named/test/TestNamedObjectStress.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : NamedObject::create( node_ + std::to_string(i), current)
SUT0 --> Runner : return / state updated
end
Runner -> Runner : ASSERT_NE(leaf, nullptr)
Runner -> Runner : EXPECT_EQ(leaf->getName(), node_999 )
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `NamedObject::create("node_" + std::to_string(i), current)`

## Verification Assertions

- `ASSERT_NE(leaf, nullptr)`
- `EXPECT_EQ(leaf->getName(), "node_999")`

---

[Back to `named` Tests Overview](../named.md)
