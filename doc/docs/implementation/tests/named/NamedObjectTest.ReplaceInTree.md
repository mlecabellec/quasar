# Test Case: `NamedObjectTest.ReplaceInTree`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 282–307) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.ReplaceInTree` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

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
Runner -> SUT0 : std::shared_ptr<NamedObject> grandchild = NamedObject::create( gr...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child2 = NamedObject::create( child2...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> replacement = NamedObject::create( r...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : child1->replaceInTree(replacement)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(replacement->getParent(), root)
Runner -> Runner : EXPECT_EQ(replacement->getChildren().size(), 1)
Runner -> Runner : EXPECT_EQ(replacement->getFirstChild(), grandchild)
Runner -> Runner : EXPECT_EQ(grandchild->getParent(), replacement)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedObject> child1 = NamedObject::create("child1", root)`
- `std::shared_ptr<NamedObject> grandchild = NamedObject::create("grandchild", child1)`
- `std::shared_ptr<NamedObject> child2 = NamedObject::create("child2", root)`
- `std::shared_ptr<NamedObject> replacement = NamedObject::create("replacement")`
- `child1->replaceInTree(replacement)`

## Verification Assertions

- `EXPECT_EQ(replacement->getParent(), root)`
- `EXPECT_EQ(replacement->getChildren().size(), 1)`
- `EXPECT_EQ(replacement->getFirstChild(), grandchild)`
- `EXPECT_EQ(grandchild->getParent(), replacement)`
- `EXPECT_EQ(child1->getParent(), nullptr)`
- `EXPECT_EQ(child1->getChildren().size(), 0)`
- `EXPECT_EQ(root->getChildren().size(), 2)`

---

[Back to `named` Tests Overview](../named.md)
