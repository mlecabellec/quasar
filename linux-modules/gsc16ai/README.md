# GSC 16AI64SSA Linux Kernel Module

This directory contains the Linux kernel module for the General Standards Corporation (GSC) 16AI64SSA data acquisition card.

## Module Information

*   **Product:** 16AI64SSA
*   **Manufacturer:** General Standards Corporation
*   **Description:** This is a Linux driver for the 16AI64SSA, a 16-bit, 64-channel, simultaneous sampling analog input card.
*   **Maintainer (likely):** ArianeGroup (based on the copyright of the associated user-space API).

## Hardware Overview

The GSC 16AI64SSA is a high-performance data acquisition card designed for PCI/PCIe based systems. It provides 64 channels of simultaneous analog input, making it suitable for a wide range of data acquisition and control applications.

## Original Source and Documentation

The original source code for this driver is provided in the `16ai64ssa.linux.3.12.111.50.0.tar.gz` archive. This archive also contains:

*   **User Manual:** `16ai64ssa_linux_um.pdf` (inside the `16ai64ssa` directory after extraction)
*   **User-space API:** A library for communicating with the driver from user-space applications.
*   **Sample Applications:** A collection of example programs demonstrating how to use the API and driver.

For more information about the hardware, refer to the General Standards Corporation website:

*   **Website:** [https://www.generalstandards.com/](https://www.generalstandards.com/)

## Maintenance Notes

*   The driver source code is located in the `driver` directory within the extracted tarball.
*   The `release.txt` file provides a detailed revision history of the driver.
*   The `gsc16ai_api` directory contains a user-space API and test suite for this driver, which is maintained by ArianeGroup.
*   The driver is designed to be used with the `gsc16ai_api` to provide a complete solution for using the 16AI64SSA card.
