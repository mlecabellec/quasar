# Test Case: `resoem_eni_generator_comparator`

| Property | Value |
| :--- | :--- |
| **Module** | `resoem` |
| **Source File** | `cmake-projects/resoem/test/test_eni_generator_comparator.cpp` (lines 1–181) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Unit tests for EniGenerator and EniComparator engines.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "VirtualEtherCATBus" as SUT0
participant "SimulatedSlave" as SUT1
participant "SM2" as SUT2
participant "SM3" as SUT3
Runner -> SUT0 : std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::cre...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create(
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create(
SUT2 --> Runner : return / state updated
Runner -> SUT3 : std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create(
SUT3 --> Runner : return / state updated
Runner -> SUT0 : s1->write_register(0x0810, 0x00)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : s1->write_register(0x0811, 0x0F)
SUT1 --> Runner : return / state updated
@enduml
```

## Observed Operations

- `std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create()`
- `std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create(`
- `std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create(`
- `std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create(`
- `s1->write_register(0x0810, 0x00)`
- `s1->write_register(0x0811, 0x0F)`
- `s1->write_register(0x0812, 0x01)`
- `s1->write_register(0x0813, 0x00)`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `resoem` Tests Overview](../resoem.md)
