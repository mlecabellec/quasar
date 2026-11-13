# Resoem Classes

This directory contains the implementation details of the `resoem` module (`cmake-projects/resoem/`), Quasar's clean-room C++23 EtherCAT MainDevice (Master) stack.

## Core Discovery and Configuration
- [Enumerator](Enumerator.md): Scanning the network and assigning slave addresses.
- [SlaveInfo](SlaveInfo.md): Data structures for slave identity and capabilities.
- [EniDocument](EniDocument.md): ETG.2100 ENI XML AST (`EniInitCmd`, `EniCoEInitCmd`, `ValidateRule`, `EniHotConnect`).
- [EniConfigurator](EniConfigurator.md): Automated `INIT`->`OP` state transitions and SyncManager/FMMU/DC programming.
- [EniGenerator](EniGenerator.md): Live bus ENI generation, XML export, and multi-level `EniComparator` verification.
- [HotConnectManager](HotConnectManager.md): Dynamic Hot Connect group discovery, state stepping, and disconnect fail-safe policies.
- [RedundancyManager](RedundancyManager.md): Cable redundancy loop bifurcation, payload/WKC merging, and self-healing probes.

## Communication Protocols
- [MailboxHandler](MailboxHandler.md): Low-level mailbox transport (`SM0`/`SM1`).
- [CoEHandler](CoEHandler.md): CANopen over EtherCAT (SDO) support.
- [MailboxProtocols](MailboxProtocols.md): File (`FoEHandler`), Ethernet (`EoEHandler`), and Servo (`SoEHandler`) over EtherCAT handlers.
- [ProcessImage](ProcessImage.md): Management of logical process data (PDO) for all slaves.
- [EtherCATTypes](EtherCATTypes.md): Wire structures and EtherCAT register/command definitions.

## Reflexive Service & Simulation
- [MasterServiceAndVirtual](MasterServiceAndVirtual.md): `EthercatMasterService` (`NamedService` integration) and hardware-free `VirtualEtherCATBus` / `SimulatedSlave` / `StubRawSocket`.
- [Diagnostics](Diagnostics.md): SDO abort and AL status code decoding helpers.

## Low-level Infrastructure
- [RawSocket](RawSocket.md): Linux `AF_PACKET` interface (`IRawSocket`) for raw Ethernet frames.
- [FrameBuilder](FrameBuilder.md): Construction of multi-datagram EtherCAT frames (`EtherCATFrame.hpp`).
- [EtherCATHeader](EtherCATHeader.md): Frame and datagram header definitions.
- [DatagramHeader](DatagramHeader.md): Individual datagram header details.

## Error Handling
- [SocketError](SocketError.md): Socket-level error reporting.
- [FrameError](FrameError.md): Protocol-level error reporting.
