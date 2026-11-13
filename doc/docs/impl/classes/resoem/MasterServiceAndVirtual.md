# EthercatMasterService & Virtual Bus

## EthercatMasterService
`resoem::EthercatMasterService` (`cmake-projects/resoem/include/resoem/EthercatMasterService.hpp`) integrates the EtherCAT master stack into Quasar's `quasar::named::NamedService` reflexive hierarchy (`TSK-20260410-001`).

- **Exposed Methods**: `refreshStatus`, `forceInit`, `reconfigureSlave`, `loadEni`, `exportEni`, `compareEni`.
- **Configuration**: `setInterface()`, `setRedundancy(enable, secondary_iface)`.

## Virtual EtherCAT Bus (`resoem::virtual_bus`)
Hardware-free simulation layer (`TSK-20260920-001.11`, `TSK-20260920-001.12`) under `cmake-projects/resoem/include/resoem/virtual/`:
- **`StubRawSocket`**: `IRawSocket` implementation routing Ethernet frames directly through memory instead of `AF_PACKET`.
- **`VirtualEtherCATBus`**: Ring/line bus simulator supporting `attach_slave`, `break_cable_after`, `restore_cable`, and dual-port redundant frame propagation.
- **`SimulatedSlave`**: 4 KiB ESC DPRAM, SII EEPROM, SyncManager, FMMU, and `quasar::named` `NamedObject` PDO binding (`create_from_eni`).
