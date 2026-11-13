# Test Case: `NamedObjectTest.RelatedObject`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 234–262) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.RelatedObject` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> obj1 = NamedObject::create( obj1 )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> obj2 = NamedObject::create( obj2 )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : obj1->setRelated(obj2)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : obj2.reset()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(obj1->getRelated(), nullptr)
Runner -> Runner : EXPECT_EQ(obj1->getRelated(), obj2)
Runner -> Runner : EXPECT_EQ(obj1->getRelated(), nullptr)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> obj1 = NamedObject::create("obj1")`
- `std::shared_ptr<NamedObject> obj2 = NamedObject::create("obj2")`
- `obj1->setRelated(obj2)`
- `obj2.reset()`

## Verification Assertions

- `EXPECT_EQ(obj1->getRelated(), nullptr)`
- `EXPECT_EQ(obj1->getRelated(), obj2)`
- `EXPECT_EQ(obj1->getRelated(), nullptr)`

---

[Back to `named` Tests Overview](../named.md)
