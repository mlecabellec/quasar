# UCEI PCI Linux Kernel Module

This directory contains the Linux kernel module for the Abaco Systems (formerly Condor Engineering and GE Intelligent Platforms) family of avionics interface cards.

## Module Information

*   **Product Family:** Universal Condor Engineering Interface (UCEI)
*   **Manufacturer:** Abaco Systems (formerly Condor Engineering / GE Intelligent Platforms)
*   **Description:** This is a universal Linux driver for a wide range of Abaco Systems avionics interface cards, including MIL-STD-1553 and ARINC-429 products.

## Hardware Overview

This driver supports a variety of PCI, PCIe, and other form-factor cards from Abaco Systems. Based on the associated `uceipci_api`, one of the primary targets for this driver is the RPCIe1553, a high-density PCI Express interface card for MIL-STD-1553 applications.

The driver is designed to be a universal interface, supporting many different hardware products through a common API.

## Original Source and Documentation

The original source code for this driver is provided in the `linux_bt1553_v828.tgz` archive. This archive contains a comprehensive driver package, including:

*   **Driver Source:** The kernel module source code for various Linux kernel versions.
*   **User-space API:** A library for communicating with the driver from user-space applications (BusTools API).
*   **Sample Applications:** Example programs demonstrating how to use the API and driver.
*   **Documentation:** Several PDF manuals for the hardware, API, and firmware.

For more information about the hardware and the BusTools software, refer to the Abaco Systems website:

*   **Website:** [https://www.abaco.com/](https://www.abaco.com/)

## Maintenance Notes

*   The driver source code is located in the `Condor_Engineering/Drivers` directory within the extracted tarball. The driver is split into versions for different kernel series (2.4, 2.6, 3.0).
*   The file `Condor_Engineering/Drivers/3.0/pci/uceipci.c` is the main source file for the modern version of the driver.
*   The `patches` directory contains a patch for `uceipci.c`, which should be reviewed to understand any local modifications.
*   The `uceipci_api` directory contains patches and test code for the user-space API that corresponds to this kernel module.
