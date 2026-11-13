# Test Case: `TypeBindingsTest.VariantHandling`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestTypeBindings.cpp` (lines 79–91) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TypeBindingsTest.VariantHandling` defined in `cmake-projects/scripting/test/TestTypeBindings.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedVariant" as SUT0
participant "Variant" as SUT1
participant "VariantType" as SUT2
Runner -> SUT0 : sol::state lua = engine->getState()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : auto v = NamedVariant::create( v , coretypes::Variant(), root)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : lua.script(R (
SUT2 --> Runner : return / state updated
Runner -> SUT0 : coretypes::Variant content = v->getVariant()
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_EQ(content.getVariantType(), coretypes::VariantType::String)
Runner -> Runner : EXPECT_EQ(content.getAs<std::string>(), hello )
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `auto v = NamedVariant::create("v", coretypes::Variant(), root)`
- `lua.script(R"(`
- `coretypes::Variant content = v->getVariant()`

## Verification Assertions

- `ASSERT_EQ(content.getVariantType(), coretypes::VariantType::String)`
- `EXPECT_EQ(content.getAs<std::string>(), "hello")`

---

[Back to `scripting` Tests Overview](../scripting.md)
