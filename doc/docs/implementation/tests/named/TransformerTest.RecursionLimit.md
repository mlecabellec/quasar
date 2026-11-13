# Test Case: `TransformerTest.RecursionLimit`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTransformer.cpp` (lines 170–182) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TransformerTest.RecursionLimit` defined in `cmake-projects/named/test/TestTransformer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : current = NamedObject::create( node , current)
SUT0 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_THROW(transformer.transform(root), std::runtime_error)
Runner -> Runner : EXPECT_THROW(transformer.transformInPlace(root), std::runtime_error)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `current = NamedObject::create("node", current)`

## Verification Assertions

- `EXPECT_THROW(transformer.transform(root), std::runtime_error)`
- `EXPECT_THROW(transformer.transformInPlace(root), std::runtime_error)`

---

[Back to `named` Tests Overview](../named.md)
