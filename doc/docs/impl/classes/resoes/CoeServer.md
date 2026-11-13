# CoeServer

## Overview
`resoes::CoeServer` (`cmake-projects/resoes/include/resoes/CoeServer.hpp`) implements `IMailboxProtocolHandler` for `MBX_TYPE_COE`, handling CANopen over EtherCAT SDO requests, SDO Information queries (`ODList`, `OD`, `ED`), Emergency messages, and dynamic PDO size calculation (`0x1C12` / `0x1C13`).

## Key Capabilities
- **`ICoeObserver`**: Pre/post SDO write hook (`on_pre_sdo_write`, `on_post_sdo_write`) allowing application rejection via CoE SDO abort codes.
- **SDO Transfers**: Supports Expedited, Segmented, and Complete Access SDO uploads/downloads with ESM state permission checks against `EscController`.
- **Dynamic PDO Sizing**: `compute_pdo_size_bits()` computes total mapped bit widths for `SM2` (`0x1C12`) and `SM3` (`0x1C13`) before transitioning to `SAFEOP`.
