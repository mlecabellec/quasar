# Test Case: `NamedObjectTest.InvalidName`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedObject.cpp` (lines 28–43) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedObjectTest.InvalidName` defined in `cmake-projects/named/test/TestNamedObject.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : EXPECT_THROW((void)NamedObject::create( ), std::runtime_error)
Runner -> Runner : EXPECT_THROW((void)NamedObject::create( 123 ),
Runner -> Runner : EXPECT_THROW((void)NamedObject::create( invalid-name ), std::runt...
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_THROW((void)NamedObject::create(""), std::runtime_error)`
- `EXPECT_THROW((void)NamedObject::create("123"),`
- `EXPECT_THROW((void)NamedObject::create("invalid-name"), std::runtime_error)`

---

[Back to `named` Tests Overview](../named.md)
