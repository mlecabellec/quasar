# EABI Linux Kernel Module

This directory contains the "EABI" (Emulated Advanced Bus Interface) Linux kernel module. This is a composite driver that provides MIL-STD-1553 functionality using an Acromag PMC-VFX70 card.

## Module Overview

The `eabi_lkm` is not a simple driver for a single piece of hardware. Instead, it's a software stack composed of three main components:

1.  **VFX70 Driver:** The low-level driver for the Acromag PMC-VFX70, a PMC card with a Xilinx Virtex-5 FPGA.
2.  **ASDC Driver:** A modified version of the `asdc_lkm` driver (originally for SBS Technologies 1553 cards) that has been adapted to run on top of the VFX70 driver. This component provides the core MIL-STD-1553 functionality, emulating an "Advanced Bus Interface" (ABI).
3.  **CEVT Driver:** The "Concentrateur d'EVenemenTs" (Event Concentrator) pseudo-driver, which is used by the ASDC component to signal events to user-space applications.

In essence, this module uses the powerful and reconfigurable FPGA on the Acromag VFX70 to emulate a dedicated MIL-STD-1553 interface card.

## Hardware

*   **Primary Hardware:** Acromag PMC-VFX70
    *   **Vendor:** Acromag
    *   **Vendor ID:** 0x16D5
    *   **Device ID:** 0x5605
    *   **Description:** A user-configurable PMC module with a Xilinx Virtex-5 FPGA.

## Original Source and Documentation

*   **Acromag Website:** [https://www.acromag.com/](https://www.acromag.com/)
*   **VFX70 Product Page:** [https://www.acromag.com/products/io-cards/pmc-modules/pmc-vfx70-virtex-5-fpga-modules-pci-x/](https://www.acromag.com/products/io-cards/pmc-modules/pmc-vfx70-virtex-5-fpga-modules-pci-x/)

For information on the MIL-STD-1553 functionality, refer to the documentation for the `asdc_lkm` module.

## Maintenance Notes

*   This is a complex, multi-component driver. Understanding the interaction between the VFX70, ASDC, and CEVT components is crucial for maintenance.
*   The source code for each component is located in its respective subdirectory under `src/`.
*   The `VFX70` driver is the primary entry point and manages the hardware directly.
*   The `ASDC` driver contains the logic for MIL-STD-1553 communication.
*   The `CEVT` driver handles event signaling.
*   The file `VFX70EMUABI.mcs` in `etc/drast_lkm/` is likely a firmware file for the Xilinx FPGA on the VFX70 card.
