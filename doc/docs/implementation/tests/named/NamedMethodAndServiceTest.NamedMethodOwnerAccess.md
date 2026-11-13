# Test Case: `NamedMethodAndServiceTest.NamedMethodOwnerAccess`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedMethodAndService.cpp` (lines 35–47) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedMethodAndServiceTest.NamedMethodOwnerAccess` defined in `cmake-projects/named/test/TestNamedMethodAndService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedMethod" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedInteger<int>> data = NamedInteger<int>::crea...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedMethod> method = NamedMethod::create( increm...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedInteger<int>> ownerData = std::dynamic_point...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : ownerData->setValue(ownerData->value() + 1)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : method->execute(nullptr)
SUT0 --> Runner : return / state updated
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_EQ(data->value(), 11)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedInteger<int>> data = NamedInteger<int>::create("data", 10, root)`
- `std::shared_ptr<NamedMethod> method = NamedMethod::create("increment", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `std::shared_ptr<NamedInteger<int>> ownerData = std::dynamic_pointer_cast<NamedInteger<int>>(owner->getChild("data"))`
- `ownerData->setValue(ownerData->value() + 1)`
- `method->execute(nullptr)`

## Verification Assertions

- `EXPECT_EQ(data->value(), 11)`

---

[Back to `named` Tests Overview](../named.md)
