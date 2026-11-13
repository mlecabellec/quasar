# Test Case: `ResoemResoesIntegrationTest.MasterVirtualBusAndSubDeviceCoeBridge`

| Property | Value |
| :--- | :--- |
| **Module** | `integration-tests` |
| **Source File** | `cmake-projects/integration-tests/TestIntegrationResoemResoes.cpp` (lines 38–112) |
| **Category** | Cross-Module / Third-Party Integration (`CS-0100.16`–`CS-0100.18`) |
| **Traceability** | `[TSK-20260920-001]`, `[TSK-20261113-001]`, `[FE-0040]`, `[FE-0280]`, `[CS-0010.37]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Cross-module integration test coupling `resoem` EtherCAT Master stack with `resoes` SubDevice stack (`CS-0100.16`, `CS-0100.18`).

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "VirtualEtherCATBus" as SUT0
participant "SimulatedSlave" as SUT1
participant "RxPDO" as SUT2
participant "TxPDO" as SUT3
Runner -> Logger : TestScopeGuard start (ResoemResoesIntegrationTest.MasterVir...)
loop Bounded test iteration / stress window
Runner -> SUT0 : resoem::virtual_bus::VirtualEtherCATBus::create()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : resoem::virtual_bus::SimulatedSlave::create( VirtualNode0 , 0x000...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : bus->attach_slave(vSlave)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : resoem::Result<size_t> enumCount = masterEnum.enumerate()
SUT3 --> Runner : return / state updated
Runner -> SUT0 : resoes::ObjectDictionary od = subApp.od()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : ).has_value())
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_TRUE(enumCount.has_value())
Runner -> Runner : EXPECT_EQ(enumCount.value(), 1U)
Runner -> Runner : ASSERT_TRUE(masterEnum.request_state_all(0x08U).has_value())
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `resoem::virtual_bus::VirtualEtherCATBus::create()`
- `resoem::virtual_bus::SimulatedSlave::create("VirtualNode0", 0x00000002U, 0x044C2C52U, 0x00100000U, 100U)`
- `bus->attach_slave(vSlave)`
- `resoem::Result<size_t> enumCount = masterEnum.enumerate()`
- `resoes::ObjectDictionary& od = subApp.od()`
- `}).has_value())`
- `}).has_value())`
- `}).has_value())`

## Verification Assertions

- `ASSERT_TRUE(enumCount.has_value())`
- `EXPECT_EQ(enumCount.value(), 1U)`
- `ASSERT_TRUE(masterEnum.request_state_all(0x08U).has_value())`
- `ASSERT_TRUE(subApp.init().has_value())`
- `ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{`
- `ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{`
- `ASSERT_TRUE(od.register_object(resoes::OdObjectEntry{`
- `EXPECT_EQ(rxBusFrame.size(), fb.buffer().size())`
- `ASSERT_TRUE(subApp.coe().unpack_rx_pdo(std::as_bytes(std::span<const resoem::byte>(frameBytes))).has_value())`
- `ASSERT_TRUE(stored.has_value())`

---

[Back to `integration-tests` Tests Overview](../integration-tests.md)
