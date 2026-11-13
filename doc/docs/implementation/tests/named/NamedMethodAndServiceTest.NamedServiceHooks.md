# Test Case: `NamedMethodAndServiceTest.NamedServiceHooks`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedMethodAndService.cpp` (lines 71–99) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedMethodAndServiceTest.NamedServiceHooks` defined in `cmake-projects/named/test/TestNamedMethodAndService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedService" as SUT0
participant "NamedMethod" as SUT1
Runner -> SUT0 : std::shared_ptr<NamedService> service = NamedService::create( hoo...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedMethod::create( configure , (std::shared_ptr<NamedObject> ow...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : NamedMethod::create( onStart , (std::shared_ptr<NamedObject> owne...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedMethod::create( onStop , (std::shared_ptr<NamedObject> owner...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : service->start()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->stop()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(configured)
Runner -> Runner : EXPECT_TRUE(started)
Runner -> Runner : EXPECT_TRUE(stopped)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedService> service = NamedService::create("hookService", root)`
- `NamedMethod::create("configure", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `NamedMethod::create("onStart", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `NamedMethod::create("onStop", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `service->start()`
- `service->stop()`

## Verification Assertions

- `EXPECT_TRUE(configured)`
- `EXPECT_TRUE(started)`
- `EXPECT_TRUE(stopped)`

---

[Back to `named` Tests Overview](../named.md)
