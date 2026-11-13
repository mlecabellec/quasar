# Test Case: `NamedMethodAndServiceTest.MultiThreadStress`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestNamedMethodAndService.cpp` (lines 101–136) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `NamedMethodAndServiceTest.MultiThreadStress` defined in `cmake-projects/named/test/TestNamedMethodAndService.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedService" as SUT0
participant "NamedMethod" as SUT1
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedService> service = NamedService::create( ser...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : NamedMethod::create( run , (std::shared_ptr<NamedObject> owner, s...
SUT1 --> Runner : return / state updated
Runner -> SUT0 : service->setCycleTime(std::chrono::milliseconds(1))
SUT0 --> Runner : return / state updated
Runner -> SUT1 : services.push_back(service)
SUT1 --> Runner : return / state updated
Runner -> SUT0 : (*it)->start()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : (*it)->stop()
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_GT(totalRuns.load(), NUM_SERVICES * 10)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedService> service = NamedService::create("service_" + std::to_string(i), root)`
- `NamedMethod::create("run", [&](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {`
- `service->setCycleTime(std::chrono::milliseconds(1))`
- `services.push_back(service)`
- `(*it)->start()`
- `(*it)->stop()`

## Verification Assertions

- `EXPECT_GT(totalRuns.load(), NUM_SERVICES * 10)`

---

[Back to `named` Tests Overview](../named.md)
