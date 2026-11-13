# Test Case: `TraversalTest.DeepCopy`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTraversal.cpp` (lines 90–155) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TraversalTest.DeepCopy` defined in `cmake-projects/named/test/TestTraversal.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : NamedInteger<int>::create( c1 , 42, root)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> childCopy = copy->getFirstChild()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> c2 = NamedObject::create( c2 , copy)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_NE(copy, root)
Runner -> Runner : EXPECT_EQ(copy->getName(), root )
Runner -> Runner : EXPECT_EQ(copy->getChildren().size(), 1)
Runner -> Runner : EXPECT_NE(childCopy, c1)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `NamedInteger<int>::create("c1", 42, root)`
- `std::shared_ptr<NamedObject> childCopy = copy->getFirstChild()`
- `std::shared_ptr<NamedObject> c2 = NamedObject::create("c2", copy)`

## Verification Assertions

- `EXPECT_NE(copy, root)`
- `EXPECT_EQ(copy->getName(), "root")`
- `EXPECT_EQ(copy->getChildren().size(), 1)`
- `EXPECT_NE(childCopy, c1)`
- `EXPECT_EQ(childCopy->getName(), "c1")`
- `ASSERT_TRUE(intCopy != nullptr)`
- `EXPECT_EQ(intCopy->toInt(), 42)`
- `EXPECT_EQ(copy->getChildren().size(), 2)`
- `EXPECT_EQ(root->getChildren().size(), 1)`

---

[Back to `named` Tests Overview](../named.md)
