# EniDocument & AST Types

## Overview
`resoem::eni::EniDocument` (`cmake-projects/resoem/include/resoem/EniDocument.hpp` and `EniConstants.hpp`) defines the typed C++23 Abstract Syntax Tree (AST) and parser for ETG.2100 EtherCAT Network Information (ENI) XML files (`TSK-20260920-001`, `FE-0040`).

## Key AST Structures
- **`ValidateRule`**: InitCmd register validation rule (`signed_val`, `type`, `data`, `data_mask`, `timeout_ms`).
- **`EniInitCmd`**: Register-level master/slave initialization command (`transition`, `before_slave`, `cmd`, `adp`, `ado`, `addr`, `data`, `retries`, `timeout_ms`, `validate`).
- **`EniCoEInitCmd`**: CoE SDO download/upload command issued during EtherCAT State Machine transitions.
- **`EniPreviousPort` / `EniHotConnect`**: Topological upstream port wiring (`A`-`D`) and Hot Connect group identification.
- **`EniConfigurator`** / **`EniGenerator`** / **`EniComparator`** operate directly on `EniDocument`.
