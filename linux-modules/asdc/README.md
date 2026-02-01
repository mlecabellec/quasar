# ASDC Linux Kernel Module

This directory contains the Linux kernel module for the SBS Technologies (now Abaco Systems) family of MIL-STD-1553 interface cards.

## Module Information

*   **Original Author:** Anonymized
*   **Description:** This driver provides an interface to the SBS Technologies MIL-STD-1553 interface cards, such as the ABI (Advanced Bus Interface) and ASF (Advanced Single Function) series. The name "ASDC" is used throughout the source code, but the hardware is from SBS.
*   **Language:** The source code comments are primarily in French.

## Supported Hardware

This driver is designed for SBS Technologies MIL-STD-1553 interface cards, including:

*   **ABI-PMC2:** A full-function, dual-redundant MIL-STD-1553 interface that can operate as a Bus Controller (BC), up to 31 Remote Terminals (RTs), and a Bus Monitor (BM).
*   **ASF-PMC2:** A single-function, dual-redundant MIL-STD-1553 interface that can operate as a BC, RT, or BM.

The hardware uses a Texas Instruments PCI bridge chip.

*   **SBS Vendor ID:** 0x1172
*   **SBS Device ID:** 0x0003
*   **Texas Instruments Vendor ID:** 0x104C
*   **Texas Instruments Device ID:** 0xAC28

## MIL-STD-1553 Overview

MIL-STD-1553 is a serial data bus standard primarily used in military avionics. It defines the communication protocol for devices on the bus, which can be a Bus Controller (BC), a Remote Terminal (RT), or a Bus Monitor (BM). This driver enables a computer to perform any of these roles.

## Original Source and Documentation

Finding original documentation for this specific driver version may be difficult due to the age of the code and the acquisition of SBS Technologies by other companies. However, information about the hardware can be found on the Abaco Systems website (the successor to SBS Technologies).

*   **Abaco Systems Website:** [https://www.abaco.com/](https://www.abaco.com/)
*   **Product Search:** Search for "MIL-STD-1553" or specific product names like "ABI-PMC" or "ASF-PMC" on their website.

## Maintenance Notes

*   This driver is complex and has a long revision history, dating back to 1991.
*   The main driver source file is `src/asdc.c`.
*   The header file `src/asdcctl.h` contains important definitions, including the vendor and device IDs.
*   The `firmware` directory contains data files that are likely firmware for the 1553 interface cards.
*   The `test` directory contains a large number of test programs and scripts for verifying the functionality of the driver and hardware.
