# Test Case: `NamedObjectTest.SerializationYAML`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 186–205) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.SerializationYAML` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : NamedInteger<int>::create( val , 123, root)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_FALSE(yaml.empty())
Runner -> Runner : EXPECT_NE(yaml.find( name: root ), std::string::npos)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `NamedInteger<int>::create("val", 123, root)`

## Verification Assertions

- `EXPECT_FALSE(yaml.empty())`
- `EXPECT_NE(yaml.find("name: root"), std::string::npos)`

---

[Back to `named` Tests Overview](../named.md)
