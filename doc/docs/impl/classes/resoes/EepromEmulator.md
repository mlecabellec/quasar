# EepromEmulator & Storage

## Overview
`resoes::EepromEmulator` (`cmake-projects/resoes/include/resoes/EepromEmulator.hpp` and `IEepromStorage.hpp`) implements SII EEPROM read, write, and reload servicing across ESC registers `0x0502`-`0x050F`, symmetrical to SOES `esc_eep`.

## Key Components
- **`IEepromStorage`**: Abstract persistence interface (`size_bytes`, `read_bytes`, `write_bytes`, `reload`).
- **`MemoryEepromStorage`**: Thread-safe (`std::timed_mutex`) RAM-backed SII EEPROM implementation storing binary SII images and tracking `reload_count()` for verification and simulation.
- **`EepromEmulator`**: Supports `4`-byte and `8`-byte SII command reads/writes, CRC verification, and reload callbacks via `std::move_only_function` (`ReloadCallback`).
