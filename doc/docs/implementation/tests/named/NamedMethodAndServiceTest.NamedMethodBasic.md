# Test Case: `NamedMethodAndServiceTest.NamedMethodBasic`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedMethodAndService.cpp` (lines 20–33) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedMethodAndServiceTest.NamedMethodBasic` defined in `cmake-projects/named/test/TestNamedMethodAndService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedMethod" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedMethod> method = NamedMethod::create( testMe...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : int input = val val->value() : 0
SUT0 --> Runner : return / state updated
Runner -> SUT0 : return NamedInteger<int>::create( result , input * 2)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedInteger<int>> args = NamedInteger<int>::crea...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> result = method->execute(args)
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(resultInt, nullptr)
Runner -> Runner : EXPECT_EQ(resultInt->value(), 42)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedMethod> method = NamedMethod::create("testMethod", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `int input = val ? val->value() : 0`
- `return NamedInteger<int>::create("result", input * 2)`
- `std::shared_ptr<NamedInteger<int>> args = NamedInteger<int>::create("args", 21)`
- `std::shared_ptr<NamedObject> result = method->execute(args)`

## Verification Assertions

- `ASSERT_NE(resultInt, nullptr)`
- `EXPECT_EQ(resultInt->value(), 42)`

---

[Back to `named` Tests Overview](../named.md)
