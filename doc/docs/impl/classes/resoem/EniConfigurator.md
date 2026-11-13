# EniConfigurator

## Overview
`resoem::eni::EniConfigurator` (`cmake-projects/resoem/include/resoem/EniConfigurator.hpp`) executes ETG.2100 ENI configuration sequences over any `IRawSocket` (`RawSocket` or `StubRawSocket`).

## Key Features
- **`configure_all()`**: Transitions the bus from `INIT` to `OP`, executing master and slave `InitCmd`s across `IP`, `PS`, `SO`, and `OP`.
- **`run_transition(EsmTransition)`**: Executes commands for a targeted ESM transition, checking `<Validate>` masks with configurable retries.
- **`configure_master_init_cmds()` / slave configuration**: Applies SyncManagers, FMMUs, Distributed Clocks, and produces detailed `TransitionReport` diagnostics on failure (`failed_slave_idx`, `failed_command_comment`, `al_status_code`).
- Thread-safe via `std::timed_mutex` (`CS-0010.46`).
