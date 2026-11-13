# Test Case: `NamedVariantTest.Cloning`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedVariant.cpp` (lines 32–42) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies that NamedVariant correctly implements the clone operation.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedVariant" as SUT0
participant "Variant" as SUT1
Runner -> SUT0 : std::shared_ptr<NamedVariant> v1 = NamedVariant::create( v1 , Var...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedVariant> v2 = std::dynamic_pointer_cast<Name...
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(v2, nullptr)
Runner -> Runner : EXPECT_EQ(v2->getName(), v1 )
Runner -> Runner : EXPECT_DOUBLE_EQ(v2->getVariant().getAs<double>(), 3.14)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedVariant> v1 = NamedVariant::create("v1", Variant(3.14))`
- `std::shared_ptr<NamedVariant> v2 = std::dynamic_pointer_cast<NamedVariant>(v1->clone())`

## Verification Assertions

- `ASSERT_NE(v2, nullptr)`
- `EXPECT_EQ(v2->getName(), "v1")`
- `EXPECT_DOUBLE_EQ(v2->getVariant().getAs<double>(), 3.14)`

---

[Back to `named` Tests Overview](../named.md)
