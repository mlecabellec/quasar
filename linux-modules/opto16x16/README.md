# GSC OPTO16X16 Linux Kernel Module

This directory contains the Linux kernel module for the General Standards Corporation (GSC) OPTO16X16 digital I/O board.

## Module Information

*   **Product:** OPTO16X16
*   **Manufacturer:** General Standards Corporation
*   **Description:** This is a Linux driver for the GSC OPTO16X16, a PCIe board with 16 optically-isolated digital inputs and 16 optically-isolated digital outputs.

## Hardware Overview

The GSC OPTO16X16 is a PCIe card designed for digital I/O applications requiring high electrical isolation. Key features include:

*   **Optically Isolated I/O:** 16 digital inputs and 16 digital outputs, each with 5000V of electrical isolation.
*   **Change-of-State Interrupts:** The board can generate interrupts on any change of level on the input channels.
*   **Programmable Features:** Debounce rates, change-of-state detection, and interrupts are software-programmable.
*   **PCIe Interface:** The card uses a PCIe interface and supports DMA bus mastering.

## Original Source and Documentation

The original source code for this driver is provided in the `opto16x16.linux.2.6.104.47.0.tar.gz` archive. This archive also contains:

*   **User Manual:** `opto16x16_linux_um.pdf` (inside the `opto16x16` directory after extraction)
*   **User-space API:** A library for communicating with the driver from user-space applications.
*   **Sample Applications:** A collection of example programs demonstrating how to use the API and driver.

For more information about the hardware, refer to the General Standards Corporation website:

*   **Website:** [https://www.generalstandards.com/](https://www.generalstandards.com/)
*   **Product Page:** [https://www.generalstandards.com/product/opto16x16-32-channel-isolated-digital-io-board/](https://www.generalstandards.com/product/opto16x16-32-channel-isolated-digital-io-board/)

## Maintenance Notes

*   The driver source code is located in the `driver` directory within the extracted tarball.
*   The `release.txt` file provides a detailed revision history of the driver.
*   The driver appears to be part of a larger framework of drivers from GSC, as evidenced by the shared file structure and utility code with other GSC drivers in this repository.
