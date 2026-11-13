# Resoem Tests

This document describes the tests for the `resoem` module, which implements a clean-room EtherCAT master.

## test_broadcast_read

### Description
The `test_broadcast_read` executable verifies the basic connectivity and frame transmission/reception capabilities of the `RawSocket` and `FrameBuilder` classes. It constructs a Broadcast Read (BRD) frame and sends it to the specified network interface.

### Scenario
1.  **Setup**: Opens a raw socket on the interface provided as a command-line argument.
2.  **Execution**:
    -   Retrieves and displays the interface MAC address.
    -   Constructs an EtherCAT frame with a single Broadcast Read (BRD) datagram at address 0x0000.
    -   Sends the frame to the wire.
    -   Waits for a response (blocking read).
3.  **Verification**:
    -   Checks if data was received.
    -   Parses the received frame to extract the Working Counter (WKC).
    -   Asserts that the WKC is valid (should be > 0 if slaves are present and responding).

## test_enumeration

### Description
The `test_enumeration` executable validates the `Enumerator` class, which is responsible for scanning the network, assigning addresses, and discovering slave information (SII).

### Scenario
1.  **Setup**: Initializes `RawSocket` and `Enumerator`.
2.  **Execution**:
    -   Calls `enumerator.enumerate()` to trigger the full enumeration process.
        -   Resets slaves to INIT.
        -   Counts slaves via BRD.
        -   Assigns configured station addresses.
        -   Reads SII (EEPROM) data including Name, Vendor ID, Product Code, and SyncManagers.
        -   Maps topology.
3.  **Verification**:
    -   Displays the count of found slaves.
    -   Iterates through the discovered `SlaveInfo` list.
    -   Prints detailed information for each slave: Name, Vendor, Product, Address, Parent Index, and SyncManager configurations.

## test_coe_upload

### Description
The `test_coe_upload` executable verifies the CoE (CANopen over EtherCAT) protocol implementation, specifically the SDO Upload (Read) functionality using `CoEHandler`.

### Scenario
1.  **Setup**: Enumerates the bus to find slaves and configure mailboxes.
2.  **Execution**:
    -   Iterates through all found slaves.
    -   Checks if the slave supports mailbox communication (valid `mbx_in` and `mbx_out` sizes).
    -   **Test Case 1**: Reads SDO 0x1008:00 (Device Name) via `sdo_read`.
    -   **Test Case 2**: Reads SDO 0x1018:01 (Vendor ID) via `sdo_read`.
3.  **Verification**:
    -   Asserts that `sdo_read` returns `CoEError::Success` for supported objects.
    -   Verifies that the read data (Device Name string and Vendor ID integer) matches expected formats (implied by successful decode).
    -   Reports success or failure for each operation.

## test_diagnostics

### Description
Verifies the `Diagnostics` class functionality, including reading error counters and monitoring port status.

## test_dc_calc

### Description
Tests the Distributed Clocks (DC) calculations, including propagation delay and offset estimation.

## test_soe_framing

### Description
Verifies the framing of Servo Profile over EtherCAT (SoE) datagrams.

## test_topology

### Description
Verifies physical slave topology tree discovery (`Enumerator` port wiring and parent/child linkage).

## test_virtual_bus (`resoem::virtual_bus`)

### Description
Executes hardware-free virtual bus tests using `VirtualEtherCATBus`, `StubRawSocket`, and `SimulatedSlave` (`BRD`, station addressing, FMMU/SM mappings, SII reads, and cable break injection).

## test_eni_parser, test_eni_configurator & test_eni_generator_comparator

### Description
Validates ETG.2100 XML parsing (`EniDocument`), `INIT -> PRE_OP -> SAFE_OP -> OP` state machine orchestration (`EniConfigurator`), live bus ENI synthesis/XML serialization (`EniGenerator`), and multi-level reference vs. bus comparison (`EniComparator`).

## test_hot_connect

### Description
Verifies dynamic Hot Connect group discovery via station aliases (`0x0012`), isolated group ESM stepping, dynamic WKC adjustment, and disconnect fail-safe policies (`ZERO_FILL`, freeze, custom).

## test_redundancy & test_redundancy_loop

### Description
Verifies dual-interface cable redundancy bifurcation upon WKC loss, reverse frame merging, and automatic healing probe recovery (`RedundancyManager`).

## test_compliance_and_stress

### Description
Exercises error handling, timeout bounds, bounded loop limits (`CS-0010.37`), concurrent `std::timed_mutex` stress tests, and compliance checks across `resoem` components.
