# Test Case: `TestRawEthernetService.VerifyPermissionSkippingOnStart`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestRawEthernetService.cpp` (lines 72–93) |
| **Category** | Unit & Regression Verification |
| **Traceability** | `[TSK-20260529-001.1]` |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Ensures socket initialization fails gracefully or skips if lacking permissions.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "RawEthernetService" as SUT0
participant "FAIL" as SUT1
Runner -> SUT0 : std::shared_ptr<RawEthernetService> service = RawEthernetService:...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : service->start()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : service->stop()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::string err = e.what()
SUT1 --> Runner : return / state updated
Runner -> SUT0 : err.find( Operation not permitted ) = std::string::npos)
SUT0 --> Runner : return / state updated
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_NE(service, nullptr)
Runner -> Runner : EXPECT_TRUE(service->isRunning())
Runner -> Runner : EXPECT_FALSE(service->isRunning())
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<RawEthernetService> service = RawEthernetService::create("ethService")`
- `service->start()`
- `service->stop()`
- `std::string err = e.what()`
- `err.find("Operation not permitted") != std::string::npos) {`

## Verification Assertions

- `ASSERT_NE(service, nullptr)`
- `EXPECT_TRUE(service->isRunning())`
- `EXPECT_FALSE(service->isRunning())`

---

[Back to `named` Tests Overview](../named.md)
