# Test Case: `OpcUaFuzzStressTest.DeterministicVariantFuzzAndConcurrentTaskQueue`

| Property | Value |
| :--- | :--- |
| **Module** | `opcua` |
| **Source File** | `cmake-projects/opcua/test_cs0100/TestOpcUaExtended.cpp` (lines 234–313) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
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
participant "UA_Variant_isEmpty" as SUT0
participant "UA_Variant_clear" as SUT1
participant "NamedString" as SUT2
participant "OpcUaServerService" as SUT3
Runner -> Logger : TestScopeGuard start (OpcUaFuzzStressTest.DeterministicVari...)
loop Bounded test iteration / stress window
Runner -> SUT0 : int kind = rng.nextInt(0, 2)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : named::NamedInteger<int32_t>::create( i , rng.nextInt(-10000, 100...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : named::NamedFloatingPoint<double>::create( d , rng.nextDouble(-50...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : named::NamedString::create( s , rng.nextAsciiString(12U))
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<OpcUaServerService> server = OpcUaServerService::...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : server->setPort(STRESS_OPCUA_PORT)
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : EXPECT_FALSE(UA_Variant_isEmpty( v))
Runner -> Runner : EXPECT_FALSE(UA_Variant_isEmpty( v))
Runner -> Runner : EXPECT_FALSE(UA_Variant_isEmpty( v))
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `int kind = rng.nextInt(0, 2)`
- `named::NamedInteger<int32_t>::create("i", rng.nextInt(-10000, 10000))`
- `named::NamedFloatingPoint<double>::create("d", rng.nextDouble(-500.0, 500.0))`
- `named::NamedString::create("s", rng.nextAsciiString(12U))`
- `std::shared_ptr<OpcUaServerService> server = OpcUaServerService::create("StressSrv")`
- `server->setPort(STRESS_OPCUA_PORT)`
- `server->start()`
- `(void)waitCv.wait_for(waitLock, SERVER_STARTUP_WAIT_MS)`

## Verification Assertions

- `EXPECT_FALSE(UA_Variant_isEmpty(&v))`
- `EXPECT_FALSE(UA_Variant_isEmpty(&v))`
- `EXPECT_FALSE(UA_Variant_isEmpty(&v))`
- `EXPECT_EQ(fut.get(), static_cast<int>((w * 100U) + t))`

---

[Back to `opcua` Tests Overview](../opcua.md)
