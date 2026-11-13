# FoE, EoE & SoE Handlers

## Overview
Additional ETG.1000 mailbox protocol handlers complementing `CoEHandler`:

- **`FoEHandler`** (`FoEHandler.hpp`): File over EtherCAT firmware and configuration transfers (`write_file`, `read_file`) with password support and bounded `std::timed_mutex` locking.
- **`EoEHandler`** (`EoEHandler.hpp`): Ethernet over EtherCAT tunneling (`send_frame`, `receive_frame`) with multi-mailbox fragmentation and reassembly.
- **`SoEHandler`** (`SoEHandler.hpp`): Servo over EtherCAT IDN read/write operations (`SoEOpCode`, `SoEElementFlags`, `SoEError`).
