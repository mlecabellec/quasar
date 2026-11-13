# Resoes Classes

This directory documents the `resoes` module (`cmake-projects/resoes/`), Quasar's C++23 clean-room EtherCAT SubDevice (Slave) stack symmetrical to SOES (`TSK-20261113-001`). Unlike global-state C firmware stacks, `resoes` encapsulates all EtherCAT Slave Controller (ESC), Mailbox, CoE, EEPROM, and cyclic PDO logic inside thread-safe, context-owned C++23 classes using `std::expected<T, SlvError>` (`Result<T>`) and `std::timed_mutex`.

## Core Infrastructure & Constants
- [EscTypes & Constants](EscTypes.md): `ResoesConstants.hpp`, `EscTypes.hpp`, `common.hpp`, `AlState`, `SyncManagerDesc`, `MailboxHeader`, `SlvError`, and runtime `SlaveConfig`.

## State Machine, Mailbox & Protocol Servers
- [EscController](EscController.md): Context-owned EtherCAT State Machine (ESM), SM0-SM3 validation, AL Error handshake, Watchdog, and DC sync monitoring (`IEsmHooks`).
- [MailboxServer](MailboxServer.md): Pooled ETG.1000 mailbox buffer state machine (`MailboxSlotState`) and `IMailboxProtocolHandler` multiplexer.
- [ObjectDictionary](ObjectDictionary.md): Type-safe, mutex-guarded CoE Object Dictionary storage (`OdObjectType`, `OdDataType`, `OdSubindexEntry`, `od_access`).
- [CoeServer](CoeServer.md): CANopen over EtherCAT server handling Expedited/Segmented/Complete-Access SDO, SDO Info, `ICoeObserver`, and dynamic `0x1C12`/`0x1C13` PDO size computation.
- [EepromEmulator](EepromEmulator.md): SII EEPROM 4-byte/8-byte read/write/reload controller (`IEepromStorage`, `MemoryEepromStorage`, `EepromEmulator`).

## Hardware Abstraction & Runtime
- [IEscHardware](IEscHardware.md): Pure virtual PDI-side HAL for ESC registers and DPRAM.
- [MemoryEscHardware](MemoryEscHardware.md): Thread-safe 8 KiB dual-port RAM simulator (`ecat_read`, `ecat_write`, `read`, `write`) enabling hardware-free unit and integration tests.
- [SlaveApplication](SlaveApplication.md): Top-level SubDevice coordinator (`init`, `step_freerun`, `IoProcessFlags`, `IApplicationCallbacks`).
