# RedundancyManager

## Overview
`resoem::redundancy::RedundancyManager` (`cmake-projects/resoem/include/resoem/RedundancyManager.hpp` and `RedundancyConstants.hpp`) implements EtherCAT Cable Redundancy over primary and secondary `IRawSocket` ports (`TSK-20260920-001.5`..`.7`).

## Key Capabilities
- **State Machine (`RedundancyState`)**: Starts in `CIRCULAR_NORMAL` and bifurcates frame transmission across primary and secondary interfaces upon WKC loss.
- **`exchange_cyclic_frame()`**: Transmits cyclic process data frames, detects cable breaks (`break_count()`), merges split secondary payloads/WKCs, and triggers healing probes (`set_probe_interval_cycles()`, `check_healing_probe()`, `heal_count()`).
- Protected by `std::timed_mutex` (`CS-0010.46`).
