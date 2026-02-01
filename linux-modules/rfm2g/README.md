# GE RFM2g Linux Kernel Module

This directory contains the Linux kernel module for the GE Intelligent Platforms (now Abaco Systems) RFM2g family of Reflective Memory devices.

## Module Information

*   **Product:** RFM2g
*   **Manufacturer:** GE Intelligent Platforms, Inc. (now part of Abaco Systems)
*   **Description:** This is a Linux driver for the GE RFM2g family of 2 Gbit/s Reflective Memory hardware.

## Hardware Overview

Reflective Memory (RFM) is a high-speed networking technology that allows multiple computers to share a common memory space over a fiber-optic ring. Each computer on the network has a local copy of the shared memory, and any changes made to the memory on one computer are automatically "reflected" to all other computers on the network.

The RFM2g family of products supports a 2 Gbit/s data rate and is available for various bus architectures, including PCI, PMC, PCIe, and VME.

## Original Source and Documentation

The original source code for this driver is provided in the `162-RFM2G-DRV-LNX-R09_00-000.tgz` archive. This archive also contains:

*   **User Manual:** `rfm2g_common.pdf` (inside the `rfm2g` directory after extraction)
*   **User-space API:** A library for communicating with the driver from user-space applications.
*   **Sample Applications:** A collection of example programs demonstrating how to use the API and driver.
*   **Diagnostic Utilities:** Tools for testing and diagnosing the RFM2g hardware.

For more information about the hardware, refer to the Abaco Systems website:

*   **Website:** [https://www.abaco.com/](https://www.abaco.com/)
*   **Product Page:** [https://www.abaco.com/products/rfm2g-family](https://www.abaco.com/products/rfm2g-family)

## Maintenance Notes

*   The driver source code is located in the `driver` directory within the extracted tarball.
*   The `release.txt` file provides a detailed revision history of the driver.
*   The `patches` directory contains patches that have been applied to the original driver source code.
*   The `rfm2g_api` directory in the parent directory may contain a user-space API that corresponds to this kernel module.
