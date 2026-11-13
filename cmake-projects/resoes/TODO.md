# resoes Development Roadmap

## Completed (Phases 1–2)
- [x] CMake module registration and `quasar_apply_standards()` integration.
- [x] `common.hpp`, `ResoesConstants.hpp`, `EscTypes.hpp` (zero C unions, `static_assert` wire checks).
- [x] `IEscHardware` + thread-safe `MemoryEscHardware` (4 KiB dual-port ESC + SM buffers + AL Events).
- [x] `IEepromStorage` + `EepromEmulator` (4-byte/8-byte read, write, reload, CRC status).
- [x] `EscController` (ESM INIT/PRE_OP/BOOT/SAFE_OP/OP, AL Error/ACK, SM0–3 validation, Watchdog, DC).
- [x] `MailboxServer` (buffer pool, SM0/SM1 state machine, `MBX_error`, repeat toggle recovery).
- [x] `ObjectDictionary` + `CoeServer` (SDO Expedited/Segmented/Complete Access, SDO Info, dynamic `0x1C12`/`0x1C13`, bit-sliced TxPDO/RxPDO pack/unpack).
- [x] `SlaveApplication` runners + hardware-free unit test suites (`test_resoes_esm_eeprom`, `test_resoes_coe_pdo`).

## Planned (Phases 3–5)
- [ ] **Phase 3**: `FoeServer` (`RRQ`/`WRQ`/`DATA`/`ACK`/`ERR`/`BUSY`, BOOT enforcement) and `EoeServer` (IP parameters, address filter, fragmented Ethernet TX/RX).
- [ ] **Phase 4**: `LinuxLan9252Hardware` (`/dev/lan9252` & SPI CSR/PRAM HAL) and sample applications (`virtual_subdevice_demo`, `linux_lan9252_subdevice`).
- [ ] **Phase 5**: `EthercatSubdeviceService` (`quasar::named::NamedService` + `ObjectDictionary` <-> `NamedObject` reflection) and bidirectional `resoem::VirtualEtherCATBus` <-> `resoes` loopback tests.
