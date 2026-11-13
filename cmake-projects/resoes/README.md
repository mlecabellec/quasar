# resoes (EtherCAT SubDevice Stack for Quasar)

`resoes` is a clean-room, C++23 EtherCAT SubDevice (slave) stack built symmetrically to `cmake-projects/resoem` and architecturally aligned with ETG.1000 / ETG.1020 specifications and SOES 3.0.0 patterns.

## Core Modules
- **`IEscHardware` / `MemoryEscHardware`**: Address-offset ESC dual-port RAM hardware abstraction and deterministic in-memory simulator.
- **`EepromEmulator`**: 4-byte / 8-byte SII EEPROM emulation (`0x0500`–`0x0515`).
- **`EscController`**: Context-owned EtherCAT State Machine (ESM), SyncManager SM0–SM3 checks, AL Status/Error ACK handshake, Watchdog, and DC Sync checks.
- **`MailboxServer`**: Pooled mailbox buffer engine implementing ETG.1000 SM0/SM1 handshakes, repeat requests, and protocol multiplexing (`xOE`).
- **`ObjectDictionary` & `CoeServer`**: Type-safe Object Dictionary, SDO Expedited/Segmented/Complete-Access Upload & Download, SDO Information (`ODList`, `OD`, `ED`), Emergency emission, and dynamic bit-sliced PDO pack/unpack (`0x1C12`/`0x1C13`).
- **`SlaveApplication`**: Deterministic free-run, synchronized poll, and interrupt-worker execution runners (`step_freerun`, `step_poll`, `handle_interrupt_events`).

See [TSK-20261113-001](../../doc/docs/project/tasks/TSK-20261113-001.md) and [TODO.md](TODO.md) for roadmap and verification status.
