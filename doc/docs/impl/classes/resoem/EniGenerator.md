# EniGenerator & EniComparator

## Overview
`EniGenerator` and `EniComparator` (`cmake-projects/resoem/include/resoem/EniGenerator.hpp` and `EniComparator.hpp`) automate ETG.2100 ENI extraction and multi-level topology verification (`TSK-20260920-001.3`, `TSK-20260920-001.4`).

## EniGenerator
- **`generate(master_name)`**: Probes slave SyncManagers, FMMUs, port descriptors (`probe_slave_topology`), and CoE PDO dictionaries (`0x1C12`/`0x1C13`, `0x1600`/`0x1A00`) to synthesize a complete `EniDocument`.
- **`serialize_to_xml_string()` / `save_to_xml_file()`**: Exports ETG.2100-compliant ENI XML.

## EniComparator
- **`compare_documents()` / live bus comparison**: Verifies candidates against golden ENI files across five `ComparisonLevel` tiers (`TOPOLOGY`, `TYPE`, `MAPPING`, `HOT_CONNECT`, `ALL`), producing a `ComparisonReport` of `Mismatch` entries with `format_summary()`.
