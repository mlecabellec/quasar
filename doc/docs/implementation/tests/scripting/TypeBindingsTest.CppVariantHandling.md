# Test Case: `TypeBindingsTest.CppVariantHandling`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestTypeBindings.cpp` (lines 93–104) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TypeBindingsTest.CppVariantHandling` defined in `cmake-projects/scripting/test/TestTypeBindings.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedVariant" as SUT0
participant "Variant" as SUT1
Runner -> SUT0 : sol::state lua = engine->getState()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : auto v = NamedVariant::create( v , coretypes::Variant(), root)
SUT1 --> Runner : return / state updated
Runner -> SUT0 : v->setVariant(coretypes::Variant(std::string( CppContent )))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sol::protected_function_result result = lua.script( return v:getV...
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(result.valid())
Runner -> Runner : EXPECT_EQ(name, CppContent )
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `auto v = NamedVariant::create("v", coretypes::Variant(), root)`
- `v->setVariant(coretypes::Variant(std::string("CppContent")))`
- `sol::protected_function_result result = lua.script("return v:getValue()")`

## Verification Assertions

- `ASSERT_TRUE(result.valid())`
- `EXPECT_EQ(name, "CppContent")`

---

[Back to `scripting` Tests Overview](../scripting.md)
