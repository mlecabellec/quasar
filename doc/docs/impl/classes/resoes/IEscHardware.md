# IEscHardware

## Overview
`resoes::IEscHardware` (`cmake-projects/resoes/include/resoes/IEscHardware.hpp`) is the pure-virtual RAII Hardware Abstraction Layer (HAL) decoupling `resoes` protocol logic from physical ESC buses (e.g., SPI/FSMC LAN9252) or simulated RAM backends.

## Interface Contract
- **`init()` / `reset()`**: Hardware lifecycle management returning `Result<>` (`std::expected<void, SlvError>`).
- **`read()` / `write()`**: Bounds-checked DPRAM and register transfers using `std::span<byte>` and `std::span<const byte>` (`CS-0020.47`).
- **`read_u16()` / `write_u16()`**: Little-endian 16-bit ESC register helpers.
- All implementations must enforce a `noexcept` virtual destructor (`CS-0060.5`).
