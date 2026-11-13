# EscController

## Overview
`resoes::EscController` (`cmake-projects/resoes/include/resoes/EscController.hpp`) manages the EtherCAT State Machine (ESM), SyncManager configuration and activation (`SM0`-`SM3`), AL Error / Error ACK handshakes, process-data Watchdog expiry detection, and Distributed Clocks (DC) synchronization status for a SubDevice instance.

## Key Types
- **`IEsmHooks`**: Observer interface invoked during transitions and PDO sizing (`on_configure_pdo_sizes`, `on_preop_transition`, `on_safeop_transition`, `on_op_transition`, `on_safe_output`).
- **`EscStateSnapshot`**: Thread-safe snapshot of `al_state`, `al_error`, `al_error_flag`, `station_address`, `dl_status`, `al_event`, `rx_pdo_bytes`, `tx_pdo_bytes`, SM active flags (`active_mbx0`, `active_mbx1`, `active_sm2`, `active_sm3`), `boot_active`, `dc_sync_enabled`, `dc_sync_mode`, and `watchdog_expired`.

## Compliance Notes
- Eliminates SOES global `ESCvar` state by owning all controller state inside each `EscController` instance.
- Uses `std::timed_mutex` (`CS-0010.46`) and `Result<T>` error propagation (`CS-0020.48`).
