# Resoes Tests

This document describes the automated unit and integration tests for the `resoes` EtherCAT SubDevice (Slave) stack (`cmake-projects/resoes/test/`), built around `MemoryEscHardware` and `MemoryEepromStorage` for hardware-free CI and Valgrind execution.

## test_esm_and_eeprom (`resoes_test_esm_and_eeprom`)
- Verifies `EscController` EtherCAT State Machine (ESM) transitions (`INIT`, `PRE_OP`, `BOOT`, `SAFE_OP`, `OP`), SyncManager (`SM0`-`SM3`) validation, AL Error / Error ACK handshakes, process-data Watchdog timeout -> safe-output fallback, and Distributed Clocks sync status checks.
- Verifies `EepromEmulator` 4-byte/8-byte SII reads, writes, CRC checks, and reload callbacks via `MemoryEepromStorage`.

## test_coe_and_pdo (`resoes_test_coe_and_pdo`)
- Verifies `MailboxServer` `SM0`/`SM1` slot transitions (`Idle`, `OwnerEcat`, `OwnerPdi`, `RepeatReq`, `Again`) and `MBX_error` handling.
- Verifies `ObjectDictionary` and `CoeServer` Expedited, Segmented, and Complete-Access SDO transfers, SDO Info (`ODList`, `OD`, `ED`), `ICoeObserver` pre/post write rejection, and dynamic `0x1C12`/`0x1C13` PDO packing/unpacking through `SlaveApplication::step_freerun()`.
