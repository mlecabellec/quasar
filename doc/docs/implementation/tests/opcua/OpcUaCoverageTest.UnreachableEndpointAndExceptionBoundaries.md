# Test Case: `OpcUaCoverageTest.UnreachableEndpointAndExceptionBoundaries`

| Property | Value |
| :--- | :--- |
| **Module** | `opcua` |
| **Source File** | `cmake-projects/opcua/test_cs0100/TestOpcUaExtended.cpp` (lines 211–232) |
| **Category** | Coverage, Enum, Boundary & Exception Containment (`CS-0100.1`, `CS-0100.3`, `CS-0100.4`, `CS-0100.6`, `CS-0100.9`) |
| **Traceability** | `[TSK-20260311-005]`, `[TSK-20261113-003]`, `[FE-0200]`, `[FE-0220]`, `[CS-0040.2]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_opcua` covering server/client mirroring, `UA_Common`, `OpcUaSecurityManager`, error boundaries, fuzzing, and concurrent client task queues.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "UA_String_fromStdString" as SUT0
participant "UA_String_clear" as SUT1
participant "UA_ByteString_fromStdString" as SUT2
participant "UA_ByteString_clear" as SUT3
Runner -> Logger : TestScopeGuard start (OpcUaCoverageTest.UnreachableEndpoint...)
Runner -> SUT0 : std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::cre...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : cli->setUrl( opc.tcp://127.0.0.1: + std::to_string(UNUSED_OPCUA_P...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : cli->stop() // Stopping an already stopped service must be an ide...
SUT2 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(emptyUaStr.length, 0U)
Runner -> Runner : EXPECT_EQ(emptyBs.length, 0U)
Runner -> Runner : EXPECT_TRUE(UA_Variant_isEmpty( nullVar))
Runner -> Runner : EXPECT_THROW(cli->start(), std::runtime_error)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::create("UnreachableClient")`
- `cli->setUrl("opc.tcp://127.0.0.1:" + std::to_string(UNUSED_OPCUA_PORT))`
- `cli->stop(); // Stopping an already stopped service must be an idempotent no-op.`

## Verification Assertions

- `EXPECT_EQ(emptyUaStr.length, 0U)`
- `EXPECT_EQ(emptyBs.length, 0U)`
- `EXPECT_TRUE(UA_Variant_isEmpty(&nullVar))`
- `EXPECT_THROW(cli->start(), std::runtime_error)`
- `EXPECT_FALSE(cli->isRunning())`

---

[Back to `opcua` Tests Overview](../opcua.md)
