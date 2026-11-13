# Test Case: `NamedObjectTest.RTTI`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 264–280) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.RTTI` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedInteger<int>> i = NamedInteger<int>::create(...
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(root->getType(), NamedObject )
Runner -> Runner : EXPECT_EQ(i->getType(), NamedInteger )
Runner -> Runner : EXPECT_TRUE(root->is<NamedObject>())
Runner -> Runner : EXPECT_FALSE(root->is<NamedInteger<int>>())
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedInteger<int>> i = NamedInteger<int>::create("intVal", 42, root)`

## Verification Assertions

- `EXPECT_EQ(root->getType(), "NamedObject")`
- `EXPECT_EQ(i->getType(), "NamedInteger")`
- `EXPECT_TRUE(root->is<NamedObject>())`
- `EXPECT_FALSE(root->is<NamedInteger<int>>())`
- `EXPECT_TRUE(i->is<NamedObject>())`
- `EXPECT_TRUE(i->is<NamedInteger<int>>())`
- `EXPECT_NO_THROW((void)i->as<NamedInteger<int>>())`

---

[Back to `named` Tests Overview](../named.md)
