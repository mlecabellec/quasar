# SlaveApplication

## Overview
`resoes::SlaveApplication` (`cmake-projects/resoes/include/resoes/SlaveApplication.hpp`) is the top-level SubDevice coordinator uniting `IEscHardware`, `IEepromStorage`, `EscController`, `EepromEmulator`, `MailboxServer`, `ObjectDictionary`, and `CoeServer`.

## Key Types & Execution
- **`IoProcessFlags`**: Fine-grained control over `process_inputs` (SM3 TxPDO), `process_outputs` (SM2 RxPDO), `check_watchdog`, and `invoke_app_hook`.
- **`IApplicationCallbacks`**: Safe application hooks (`on_rx_pdo_updated`, `on_tx_pdo_prepare`, `on_safe_outputs`, `on_application_tick`).
- **`init()` / `step_freerun()`**: Initializes the stack and drives one deterministic free-run polling cycle (handling AL Control transitions, EEPROM requests, SM0/SM1 mailbox exchanges, and cyclic SM2/SM3 PDO updates).
