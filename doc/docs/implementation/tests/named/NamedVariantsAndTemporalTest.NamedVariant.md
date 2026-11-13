# Test Case: `NamedVariantsAndTemporalTest.NamedVariant`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedVariantsAndTemporal.cpp` (lines 17–39) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies the basic functionality of the new value-based NamedVariant.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedVariant" as SUT0
participant "VariantType" as SUT1
participant "Variant" as SUT2
Runner -> SUT0 : std::shared_ptr<NamedVariant> variant = NamedVariant::create( myV...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : variant->setVariant(Variant(static_cast<int64_t>(100)))
SUT1 --> Runner : return / state updated
Runner -> SUT2 : variant->setVariant(Variant(std::string( hello )))
SUT2 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(variant->getVariantType(), VariantType::Empty)
Runner -> Runner : EXPECT_FALSE(variant->getVariant().holds<int64_t>())
Runner -> Runner : EXPECT_TRUE(variant->getVariant().holds<int64_t>())
Runner -> Runner : EXPECT_FALSE(variant->getVariant().holds<std::string>())
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedVariant> variant = NamedVariant::create("myVariant")`
- `variant->setVariant(Variant(static_cast<int64_t>(100)))`
- `variant->setVariant(Variant(std::string("hello")))`

## Verification Assertions

- `EXPECT_EQ(variant->getVariantType(), VariantType::Empty)`
- `EXPECT_FALSE(variant->getVariant().holds<int64_t>())`
- `EXPECT_TRUE(variant->getVariant().holds<int64_t>())`
- `EXPECT_FALSE(variant->getVariant().holds<std::string>())`
- `EXPECT_EQ(variant->getVariant().getAs<int64_t>(), 100)`
- `EXPECT_FALSE(variant->getVariant().holds<int64_t>())`
- `EXPECT_TRUE(variant->getVariant().holds<std::string>())`
- `EXPECT_EQ(variant->getVariant().getAs<std::string>(), "hello")`

---

[Back to `named` Tests Overview](../named.md)
