# Spectracom TSyncPCI Linux Kernel Module

This directory contains the Linux kernel module for the Spectracom TSync family of time synchronization cards.

## Module Information

*   **Product:** TSync
*   **Manufacturer:** Spectracom Corporation
*   **Description:** This is a Linux driver for the Spectracom TSync series of timing boards, which are used for high-precision time synchronization in computer systems.

## Hardware Overview

The Spectracom TSync cards are designed to provide precise time to a host computer. They can be synchronized to a variety of external time sources and can also generate time codes and other timing signals. Key features include:

*   **High Precision:** Nanosecond-level resolution.
*   **Synchronization Sources:** Can be synchronized to GPS, IRIG, 1PPS, and other time sources.
*   **Holdover Capability:** An internal oscillator maintains time when external sources are unavailable.
*   **Inputs and Outputs:** Multiple time code outputs, programmable "heartbeat" signals, and event timestamping inputs.
*   **Bus Interfaces:** Available in various form factors, including PCI, PCIe, cPCI, and more.

## Original Source and Documentation

The original source code for this driver is provided in the `tsync-4.0.5.tar.gz` archive. This archive also contains:

*   **User Manual:** `manual.txt` (inside the `tsync` directory after extraction)
*   **User-space API:** `libtsync`, a library for communicating with the driver from user-space applications.
*   **Sample Applications:** A collection of example programs demonstrating how to use the API and driver.
*   **Utilities:** Command-line utilities for configuring and testing the TSync card.

For more information about the hardware, refer to the Spectracom website (now part of Orolia).

*   **Website:** [https://www.orolia.com/](https://www.orolia.com/) (Orolia acquired Spectracom)

## Maintenance Notes

*   The driver source code is located in the `tsync-driver` directory within the extracted tarball.
*   The `tsyncpci.h` header file in the `tsync-driver` directory contains the PCI vendor and device IDs.
*   The `patches` directory contains a patch for `tsync_pps.c`, which is likely related to the Pulse Per Second (PPS) functionality of the driver.
*   The `tsyncpci_api` directory contains patches for the user-space API headers.
