# Resoem Classes

This directory contains the implementation details of the Resoem module, a clean-room EtherCAT master.

## Core Discovery and Configuration
- [Enumerator](Enumerator.md): Scanning the network and assigning slave addresses.
- [SlaveInfo](SlaveInfo.md): Data structures for slave identity and capabilities.

## Communication Protocols
- [MailboxHandler](MailboxHandler.md): Low-level mailbox transport.
- [CoEHandler](CoEHandler.md): CANopen over EtherCAT (SDO) support.
- [ProcessImage](ProcessImage.md): Management of logical process data (PDO) for all slaves.

## Low-level Infrastructure
- [RawSocket](RawSocket.md): Linux AF_PACKET interface for raw Ethernet frames.
- [FrameBuilder](FrameBuilder.md): Construction of multi-datagram EtherCAT frames.
- [EtherCATHeader](EtherCATHeader.md): Frame and datagram header definitions.
- [DatagramHeader](DatagramHeader.md): Individual datagram header details.

## Error Handling
- [SocketError](SocketError.md): Socket-level exceptions.
- [FrameError](FrameError.md): Protocol-level exceptions.
