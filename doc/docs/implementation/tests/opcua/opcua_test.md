# Test Case: `opcua_test`

| Property | Value |
| :--- | :--- |
| **Module** | `opcua` |
| **Source File** | `cmake-projects/opcua/test/TestOpcUaServer.cpp` (lines 1–208) |
| **Category** | Unit & Regression Verification |
| **Traceability** | `TSK-20261113-003,`, `FE-0140,`, `CS-0010,`, `CS-0010.21,` |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Unit tests verifying OpcUaServerService startup, address-space mapping, and method node registration.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "SetUp" as SUT0
participant "TearDown" as SUT1
participant "Allocation" as SUT2
participant "NamedObject" as SUT3
Runner -> SUT0 : * stopping it cleanly in TearDown() .
SUT0 --> Runner : return / state updated
Runner -> SUT1 : * invariant server is configured on port 4841 before each test an...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : * details Creates Root , Data , MyInt ( 42 ), and server bound to...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : * compliance Deterministic fixture setup via create() factory met...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : * - Resource / Allocation (object creation failure): Unwinds clea...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : * Limitations: Port 4841 conflict surfaces only when server->star...
SUT1 --> Runner : return / state updated
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_NO_THROW(server->start())
Runner -> Runner : EXPECT_TRUE(server->isRunning())
Runner -> Runner : EXPECT_NO_THROW(myInt->setValue(100))
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `* stopping it cleanly in `TearDown()`.`
- `* @invariant `server` is configured on port 4841 before each test and stopped in `TearDown()`.`
- `* @details Creates `Root`, `Data`, `MyInt` (`42`), and `server` bound to `root`.`
- `* @compliance Deterministic fixture setup via `create()` factory methods.`
- `*   - Resource / Allocation (object creation failure): Unwinds cleanly and aborts test.`
- `*   Limitations: Port 4841 conflict surfaces only when `server->start()` runs in test body.`
- `root = NamedObject::create("Root")`
- `data = NamedObject::create("Data", root)`

## Verification Assertions

- `EXPECT_NO_THROW(server->start())`
- `EXPECT_TRUE(server->isRunning())`
- `EXPECT_NO_THROW(myInt->setValue(100))`
- `EXPECT_NO_THROW(server->stop())`
- `EXPECT_FALSE(server->isRunning())`
- `EXPECT_NO_THROW(server->start())`
- `EXPECT_TRUE(server->isRunning())`
- `EXPECT_NO_THROW(server->stop())`

---

[Back to `opcua` Tests Overview](../opcua.md)
