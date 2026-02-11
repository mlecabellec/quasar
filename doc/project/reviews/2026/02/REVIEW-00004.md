# REVIEW-00004: Review of Resoem (EtherCAT Master library). (WIP)

## Description

This review is a preliminary review of the Resoem library.

## Review scope

### Reviewed code

- [REVIEW-00004.1] resoem library at commit [a723425c3bbdd16325457402ae73075d1ecc2c4f].

### Reviewed features and constraints

- [REVIEW-00004.2] Review of the [FE-0040] feature, including:
  - [FE-0040.1] Transport Layer (Linux Raw Sockets):
    - [FE-0040.1.1] Use Linux AF_PACKET sockets with SOCK_RAW.
    - [FE-0040.1.2] Support configurable receive timeouts and MAC retrieval.
    - [FE-0040.1.3] Implement non-blocking send/receive with exception handling.
  - [FE-0040.2] EtherCAT Frame Management:
    - [FE-0040.2.1] Provide a FrameBuilder for multi-datagram frames.
    - [FE-0040.2.2] Support standard EtherCAT commands (APRD, APWR, etc.).
    - [FE-0040.2.3] Implement "More" bit handling and frame padding.
    - [FE-0040.2.4] Support 32-bit logical addressing.
  - [FE-0040.3] Network Discovery & Enumeration:
    - [FE-0040.3.1] Automatically detect and count slaves.
    - [FE-0040.3.2] Assign configured station addresses.
    - [FE-0040.3.3] Parse SII/EEPROM (Vendor ID, Product Code, etc.).
    - [FE-0040.3.4] Map network topology and relationships.
  - [FE-0040.4] Mailbox Protocols:
    - [FE-0040.4.1] CoE (CANopen over EtherCAT): SDO transfers (expedited, normal, segmented), OD browsing.
    - [FE-0040.4.2] FoE (File over EtherCAT): upload/download state machines, Bootstrap support.
    - [FE-0040.4.3] EoE (Ethernet over EtherCAT): fragmentation/reassembly, virtual network communication.
  - [FE-0040.5] Process Data (PDO) Configuration:
    - [FE-0040.5.1] Support PDO mapping retrieval from SII and CoE.
    - [FE-0040.5.2] Automatically calculate and program FMMU entries.
    - [FE-0040.5.3] Manage global ProcessImage buffer with bit-level accessors.
    - [FE-0040.5.4] Support cyclic data exchange using LRW.
  - [FE-0040.6] Distributed Clocks (DC) Synchronization:
    - [FE-0040.6.1] Implement propagation delay measurement.
    - [FE-0040.6.2] Designate a Reference Clock.
    - [FE-0040.6.3] Provide cyclic drift compensation using ARMW.
    - [FE-0040.6.4] Configure SYNC0/SYNC1 signals.
  - [FE-0040.7] Resilience & Diagnostics:
    - [FE-0040.7.1] Implement Hot-Connect support.
    - [FE-0040.7.2] Support mailbox retry logic.
    - [FE-0040.7.3] Provide diagnostic strings for SDO Abort and AL Status codes.
  - [FE-0040.8] Modern C++23 Implementation Standards:
    - [FE-0040.8.1] Use std::expected (Result<T>) for error handling.
    - [FE-0040.8.2] Use std::span for zero-copy views.
    - [FE-0040.8.3] Leverage std::chrono for timing.
    - [FE-0040.8.4] Employ packed structures and standard-compliant attributes.

## Review results

- [REVIEW-00004.3] Initial assessment in progress.

### Issues found

### Issues resolved

### Issues not resolved

### Issues to be resolved in the future

## Conclusion

Work is in progress.
