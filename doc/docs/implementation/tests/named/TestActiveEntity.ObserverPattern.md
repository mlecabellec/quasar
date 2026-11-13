# Test Case: `TestActiveEntity.ObserverPattern`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestActiveEntity.cpp` (lines 170–190) |
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
Runner -> SUT0 : std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSe...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sensor->subscribe(observer)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sensor->initialize()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sensor->start() // This triggers notifyObservers( sensorStarted )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sensor->unsubscribe(observer)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : sensor->start() // Triggers again but observer should be removed
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(observer->notificationCount, 0)
Runner -> Runner : EXPECT_EQ(observer->notificationCount, 1)
Runner -> Runner : ASSERT_NE(observer->lastEventData, nullptr)
Runner -> Runner : EXPECT_EQ(observer->lastEventData->getName(), sensorStarted )
@enduml
```

## Observed Operations

- `std::shared_ptr<MockTemperatureSensor> sensor = MockTemperatureSensor::create("sensor1")`
- `sensor->subscribe(observer)`
- `sensor->initialize()`
- `sensor->start(); // This triggers notifyObservers("sensorStarted")`
- `sensor->unsubscribe(observer)`
- `sensor->start(); // Triggers again but observer should be removed`

## Verification Assertions

- `EXPECT_EQ(observer->notificationCount, 0)`
- `EXPECT_EQ(observer->notificationCount, 1)`
- `ASSERT_NE(observer->lastEventData, nullptr)`
- `EXPECT_EQ(observer->lastEventData->getName(), "sensorStarted")`
- `EXPECT_EQ(observer->notificationCount, 1); // No increment`

---

[Back to `named` Tests Overview](../named.md)
