# Test Case: `NamedObjectTest.Uniqueness`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 96–120) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.Uniqueness` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child1 = NamedObject::create( child ...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child2 = NamedObject::create( child )
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_THROW((void)NamedObject::create( child , root), std::runti...
Runner -> Runner : EXPECT_THROW(child2->setParent(root), std::runtime_error)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedObject> child1 = NamedObject::create("child", root)`
- `std::shared_ptr<NamedObject> child2 = NamedObject::create("child")`

## Verification Assertions

- `EXPECT_THROW((void)NamedObject::create("child", root), std::runtime_error)`
- `EXPECT_THROW(child2->setParent(root), std::runtime_error)`

---

[Back to `named` Tests Overview](../named.md)
