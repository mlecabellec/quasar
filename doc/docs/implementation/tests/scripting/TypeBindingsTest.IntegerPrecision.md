# Test Case: `TypeBindingsTest.IntegerPrecision`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestTypeBindings.cpp` (lines 31–40) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TypeBindingsTest.IntegerPrecision` defined in `cmake-projects/scripting/test/TestTypeBindings.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : sol::state lua = engine->getState()
Runner -> Runner : auto val = NamedInteger<int64_t>::create( val , 123456789012345LL...
Runner -> Runner : sol::protected_function_result result = lua.script( return val:va...
Runner -> Runner : ASSERT_TRUE(result.valid())
Runner -> Runner : EXPECT_EQ(retrieved, 123456789012345LL)
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `auto val = NamedInteger<int64_t>::create("val", 123456789012345LL, root)`
- `sol::protected_function_result result = lua.script("return val:value()")`

## Verification Assertions

- `ASSERT_TRUE(result.valid())`
- `EXPECT_EQ(retrieved, 123456789012345LL)`

---

[Back to `scripting` Tests Overview](../scripting.md)
