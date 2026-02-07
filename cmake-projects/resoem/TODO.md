# Resoem Implementation Roadmap

## Phase 1: Transport Layer & Basic Connectivity
- [x] Implement `RawSocket` class wrapping Linux `socket(AF_PACKET)`.
- [x] Implement `EtherCATFrame` builder/parser.
- [x] Create `test_broadcast_read` tool to send a BRD frame and verify WKC return.

## Phase 2: Enumeration
- [x] Implement `Enumerator` class.
- [x] Implement primitive "Broadcast Read" loop to count slaves.
- [x] Implement generic SII Category parsing (SyncManagers, FMMUs).
- [x] Topology mapping.

- [ ] Parse critical SII categories:
    - [x] General (Vendor ID, Product Code).
    - [x] Strings (Name).
    - [ ] SyncManagers.
    - [ ] FMMUs.

## Phase 3: Mailbox & CoE

## Phase 4: Process Data (PDO)
- [ ] Implement `SyncManager` configuration logic.
- [ ] Implement `FMMU` calculation and configuration.
- [ ] Allocate and manage the logical process image.
- [ ] Implement `CyclicTask` loop.

## Phase 5: Distributed Clocks (Future)
- [ ] Implement initial time distribution.
- [ ] Implement drift compensation.
