# Test Case: `resoem_hot_connect`

| Property | Value |
| :--- | :--- |
| **Module** | `resoem` |
| **Source File** | `cmake-projects/resoem/test/test_hot_connect.cpp` (lines 1–283) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Unit test verifying EtherCAT Hot Connect dynamic discovery, ESM stepping, and policy enforcement.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "EsmTransition" as SUT0
participant "DatagramCmd" as SUT1
participant "ToolArm" as SUT2
participant "VirtualEtherCATBus" as SUT3
Runner -> SUT0 : doc.master().init_cmds.push_back(master_cmd)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : doc.slaves().push_back(slave0)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : slave1.sync_managers.push_back(sm2)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : rx_pdo.entries.push_back(rx_entry)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : slave1.rx_pdos.push_back(rx_pdo)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : doc.slaves().push_back(slave1)
SUT1 --> Runner : return / state updated
@enduml
```

## Observed Operations

- `doc.master().init_cmds.push_back(master_cmd)`
- `doc.slaves().push_back(slave0)`
- `slave1.sync_managers.push_back(sm2)`
- `rx_pdo.entries.push_back(rx_entry)`
- `slave1.rx_pdos.push_back(rx_pdo)`
- `doc.slaves().push_back(slave1)`
- `rule.data = {static_cast<std::uint8_t>(TOOL_ARM_ALIAS & 0xFF),`
- `hc.identify_cmds.push_back(ident)`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `resoem` Tests Overview](../resoem.md)
