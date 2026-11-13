# Test Case: `TypeBindingsTest.HierarchyResolution`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestTypeBindings.cpp` (lines 63–77) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TypeBindingsTest.HierarchyResolution` defined in `cmake-projects/scripting/test/TestTypeBindings.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : sol::state lua = engine->getState()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> child = NamedObject::create( child ,...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> grandchild = NamedObject::create( gr...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::protected_function_result result = lua.script( return quasar...
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(result.valid())
Runner -> Runner : ASSERT_NE(resolved, nullptr)
Runner -> Runner : EXPECT_EQ(resolved->getName(), grandchild )
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `std::shared_ptr<NamedObject> child = NamedObject::create("child", root)`
- `std::shared_ptr<NamedObject> grandchild = NamedObject::create("grandchild", child)`
- `sol::protected_function_result result = lua.script("return quasar.resolve(root, 'child/grandchild')")`

## Verification Assertions

- `ASSERT_TRUE(result.valid())`
- `ASSERT_NE(resolved, nullptr)`
- `EXPECT_EQ(resolved->getName(), "grandchild")`

---

[Back to `scripting` Tests Overview](../scripting.md)
