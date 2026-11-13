# MemoryEscHardware

## Overview
`resoes::MemoryEscHardware` (`cmake-projects/resoes/include/resoes/hal/MemoryEscHardware.hpp`) is a deterministic, thread-safe dual-port ESC DPRAM simulator implementing `IEscHardware`.

## Dual-Port Architecture
- **PDI Side (`read`, `write`, `read_u16`, `write_u16`)**: Used by `EscController`, `MailboxServer`, `EepromEmulator`, and `SlaveApplication`.
- **ECAT / Master Side (`ecat_read`, `ecat_write`, `ecat_write_u16`)**: Simulates MainDevice EtherCAT frame accesses into ESC registers and SyncManager regions while raising the corresponding AL Event flags and SM activation handshakes.
- Enables deterministic closed-loop MainDevice/SubDevice unit testing without root privileges or physical NICs.
