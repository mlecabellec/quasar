# HotConnectManager

## Overview
`resoem::hot_connect::HotConnectManager` (`cmake-projects/resoem/include/resoem/HotConnectManager.hpp` and `HotConnectConstants.hpp`) enables non-intrusive insertion and removal of modular EtherCAT SubDevice groups at runtime (`TSK-20260920-001.8`, `TSK-20260920-001.9`).

## Key Capabilities
- **`HotConnectGroupConfig`**: Tracks `station_alias` (register `0x0012`), member `slave_indices`, `expected_wkc`, input process image slice (`input_process_offset`, `input_process_length`), `DisconnectedDataPolicy` (`ZERO_FILL`, freeze `last_known_valid_data`, or `custom_fallback_data`), and `HotConnectGroupState`.
- **`auto_discover_groups_from_eni()` / `register_group()`**: Populates Hot Connect groups from an `EniDocument` or explicit API calls.
- **`group_state()`**: Thread-safe group lifecycle tracking while stepping detached groups (`INIT -> PRE_OP -> SAFE_OP -> OP`) without interrupting active slaves.
