# Test Case: `NamedMethodAndServiceTest.NamedServiceLifecycle`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedMethodAndService.cpp` (lines 49–69) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedMethodAndServiceTest.NamedServiceLifecycle` defined in `cmake-projects/named/test/TestNamedMethodAndService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedService" as SUT0
participant "NamedMethod" as SUT1
Runner -> SUT0 : std::shared_ptr<NamedService> service = NamedService::create( tes...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedMethod::create( run , (std::shared_ptr<NamedObject> owner, s...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : service->setCycleTime(std::chrono::milliseconds(10))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->start()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : int finalCounter = counter.load()
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_GE(finalCounter, 3)
Runner -> Runner : EXPECT_EQ(counter.load(), finalCounter)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedService> service = NamedService::create("testService", root)`
- `NamedMethod::create("run", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `service->setCycleTime(std::chrono::milliseconds(10))`
- `service->start()`
- `service->stop()`
- `int finalCounter = counter.load()`

## Verification Assertions

- `EXPECT_GE(finalCounter, 3)`
- `EXPECT_EQ(counter.load(), finalCounter)`

---

[Back to `named` Tests Overview](../named.md)
