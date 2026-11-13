# Test Case: `TypeBindingsTest.IsolatedCreation`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestTypeBindings.cpp` (lines 106–116) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TypeBindingsTest.IsolatedCreation` defined in `cmake-projects/scripting/test/TestTypeBindings.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : sol::state lua = engine->getState()
Runner -> Runner : lua.script(R (
Runner -> Runner : local obj = quasar.named.createObject( Orphan )
Runner -> Runner : EXPECT_EQ(name, Orphan )
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `lua.script(R"(`
- `local obj = quasar.named.createObject("Orphan")`

## Verification Assertions

- `EXPECT_EQ(name, "Orphan")`

---

[Back to `scripting` Tests Overview](../scripting.md)
