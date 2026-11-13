# EscTypes, Constants & Common Primitives

## Overview
- **`ResoesConstants.hpp`** (`resoes::constants`): Symbolic constants (`UPPER_SNAKE_CASE`, `CS-0040`) for ESC register addresses, AL Status/Control bits, SyncManager registers, EEPROM control/status registers (`EEP_READ_SIZE_8`, `EEP_DEFAULT_SIZE_BYTES`), and `ALERR_NONE` / `ALERR_*` error codes.
- **`common.hpp`**: Core aliases including `resoes::byte`, `SlvError`, and `Result<T>` (`std::expected<T, SlvError>`).
- **`EscTypes.hpp`**: Wire-verified protocol structures (`static_assert(sizeof(...) == ...)` per `CS-0020.74`) without C `union`s (`CS-0060.6`), including `AlState`, `MailboxHeader`, SyncManager descriptors, and runtime `SlaveConfig`.

## Design Compliance
- Replaces SOES compile-time `#define` macros and `ESCvar` globals with runtime `SlaveConfig` and strong `enum class` / explicit byte serialization helpers.
- Enforces `[CS-0010.34]` (zero `auto`), `[CS-0020.48]` (`Result<T>`), `[CS-0020.74]` (`sizeof` static assertions), and `[CS-0060.6]` (zero C `union`s).
