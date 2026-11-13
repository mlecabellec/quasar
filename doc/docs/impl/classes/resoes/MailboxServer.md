# MailboxServer

## Overview
`resoes::MailboxServer` (`cmake-projects/resoes/include/resoes/MailboxServer.hpp`) implements the pooled ETG.1000 Mailbox state machine over `SM0` (master-to-slave out) and `SM1` (slave-to-master in) without global `MBX[]` / `MBXcontrol[]` arrays.

## Key Types
- **`MailboxSlotState`**: Enumerates slot lifecycle (`Idle`, `OwnerEcat`, `OwnerPdi`, `XoeInProgress`, `RepeatReq`, `Again`).
- **`IMailboxProtocolHandler`**: Pluggable protocol interface (`protocol_type()`, `handle_request()`, `reset()`) implemented by CoE (`CoeServer`) and future `FoE`/`EoE` servers.

## Operation
- Polls `SM0` via `IEscHardware` when active in `EscController`.
- Dispatches validated ETG.1000 frames to the matching `IMailboxProtocolHandler`, handles `MBX_error` responses for unknown/malformed protocol headers, and manages `SM1` transmission and repeat-request buffers.
