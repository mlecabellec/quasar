# Test Case: `LuaServiceTest.WorstCaseTermination`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestLuaService.cpp` (lines 96–112) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `LuaServiceTest.WorstCaseTermination` defined in `cmake-projects/scripting/test/TestLuaService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "LuaService" as SUT0
Runner -> SUT0 : std::shared_ptr<LuaService> service = LuaService::create( TestSer...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->start()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->execute( for i=1,10000000 do end )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : service.reset()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : t.join()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(true) // If we reached here without a segfault, test ...
@enduml
```

## Observed Operations

- `std::shared_ptr<LuaService> service = LuaService::create("TestService", root)`
- `service->start()`
- `service->execute("for i=1,10000000 do end")`
- `service->stop()`
- `service.reset()`
- `t.join()`

## Verification Assertions

- `EXPECT_TRUE(true); // If we reached here without a segfault, test passed`

---

[Back to `scripting` Tests Overview](../scripting.md)
