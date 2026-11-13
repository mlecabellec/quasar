# Test Case: `NamedObjectTest.Hierarchy`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 45–94) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.Hierarchy` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child1 = NamedObject::create( child1...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child2 = NamedObject::create( child2 )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : child2->setParent(root)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : child2->setParent(nullptr)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(child1->getParent(), root)
Runner -> Runner : EXPECT_EQ(root->getChildren().size(), 1)
Runner -> Runner : EXPECT_EQ(root->getFirstChild(), child1)
Runner -> Runner : EXPECT_EQ(root->getChildren().size(), 2)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedObject> child1 = NamedObject::create("child1", root)`
- `std::shared_ptr<NamedObject> child2 = NamedObject::create("child2")`
- `child2->setParent(root)`
- `child2->setParent(nullptr)`

## Verification Assertions

- `EXPECT_EQ(child1->getParent(), root)`
- `EXPECT_EQ(root->getChildren().size(), 1)`
- `EXPECT_EQ(root->getFirstChild(), child1)`
- `EXPECT_EQ(root->getChildren().size(), 2)`
- `EXPECT_EQ(child1->getNextSibling(), child2)`
- `EXPECT_EQ(child2->getPreviousSibling(), child1)`
- `EXPECT_EQ(root->getChildren().size(), 1)`
- `EXPECT_EQ(child2->getParent(), nullptr)`

---

[Back to `named` Tests Overview](../named.md)
