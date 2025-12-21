# AI64 Linux Kernel Module

This directory contains the Linux kernel module for the General Standards Corporation AI64 family of data acquisition cards.

## Module Information

*   **Original Author:** Evan Hillman, General Standards Corporation
*   **Copyright:** General Standards Corporation (GSC), 2002-2003
*   **Description:** This is a Linux host driver for the PMC-16AI64SS card and other related models.

## Supported Hardware

This driver supports a family of PMC/PCI data acquisition cards from General Standards Corporation, including:

*   PMC-12AI64
*   PMC-16AI64
*   PMC-16AI64SS
*   PMC-16AI64SSA/C

These cards are based on the PLX Technology PCI9080 and PCI9056 bridge chips.

## Original Source and Documentation

The original source code and documentation for this driver and the associated hardware can be found on the General Standards Corporation website.

*   **Website:** [https://www.generalstandards.com/](https://www.generalstandards.com/)
*   **Product Page (Example):** Search for "PMC-16AI64SS" on their website.

It is recommended to refer to the official documentation for detailed information about the hardware, driver installation, and API usage.

## Maintenance Notes

*   This driver is a modified version of the original driver from General Standards Corporation.
*   When updating or modifying this driver, it is important to consult the original source code and documentation.
*   The source code contains a detailed revision history that can be useful for understanding the changes made to the driver over time.
*   The file `ai64_lkm/src/ai64.c` contains the main driver source code.
*   The file `ai64_lkm/src/ai64_ioctl.h` defines the ioctl commands used to communicate with the driver.
*   The file `ai64_lkm/src/plx_regs.h` contains register definitions for the PLX PCI bridge chip.
