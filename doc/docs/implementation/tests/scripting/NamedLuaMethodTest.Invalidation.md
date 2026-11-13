# Test Case: `NamedLuaMethodTest.Invalidation`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestNamedLuaMethod.cpp` (lines 45–57) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Set up the test suite.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedLuaMethod" as SUT0
Runner -> SUT0 : sol::state lua = engine->getState()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sol::function func = lua.load( return function(self, args) return...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedLuaMethod> method = NamedLuaMethod::create( ...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : engine->shutdown()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> result = method->execute(nullptr)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(result, nullptr)
@enduml
```

## Observed Operations

- `sol::state& lua = engine->getState()`
- `sol::function func = lua.load("return function(self, args) return 0 end").call()`
- `std::shared_ptr<NamedLuaMethod> method = NamedLuaMethod::create("test", func, root)`
- `engine->shutdown()`
- `std::shared_ptr<NamedObject> result = method->execute(nullptr)`

## Verification Assertions

- `EXPECT_EQ(result, nullptr)`

---

[Back to `scripting` Tests Overview](../scripting.md)
