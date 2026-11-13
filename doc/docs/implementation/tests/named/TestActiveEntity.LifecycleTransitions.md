# Test Case: `TestActiveEntity.LifecycleTransitions`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestActiveEntity.cpp` (lines 151–168) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

A simple mock for event data.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "MockTemperatureSensor" as SUT0
participant "EntityState" as SUT1
Runner -> SUT0 : std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSe...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sensor->initialize()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : sensor->start()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sensor->stop()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : sensor->reset()
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(sensor->getState(), EntityState::Uninitialized)
Runner -> Runner : EXPECT_EQ(sensor->getState(), EntityState::Ready)
Runner -> Runner : EXPECT_EQ(sensor->getState(), EntityState::Running)
Runner -> Runner : EXPECT_EQ(sensor->getState(), EntityState::Ready)
@enduml
```

## Observed Operations

- `std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1")`
- `sensor->initialize()`
- `sensor->start()`
- `sensor->stop()`
- `sensor->reset()`

## Verification Assertions

- `EXPECT_EQ(sensor->getState(), EntityState::Uninitialized)`
- `EXPECT_EQ(sensor->getState(), EntityState::Ready)`
- `EXPECT_EQ(sensor->getState(), EntityState::Running)`
- `EXPECT_EQ(sensor->getState(), EntityState::Ready)`
- `EXPECT_EQ(sensor->getState(), EntityState::Uninitialized)`

---

[Back to `named` Tests Overview](../named.md)
