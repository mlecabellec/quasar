# AI64LL Linux Kernel Module

This directory contains the Linux kernel module for the Concurrent Real-Time AI64LL family of analog input cards.

## Module Information

*   **Copyright:** Concurrent Real-Time, Inc. (2003 and beyond)
*   **Description:** This is a Linux host driver for the 16AI64LL (16-bit) Analog Input card. The "LL" likely stands for "Low Latency".

## Supported Hardware

This driver is designed for the AI64LL series of data acquisition cards from Concurrent Real-Time. These cards are closely related to the General Standards Corporation AI64 series and share the same PCI vendor, device, and subsystem IDs.

The driver differentiates between the AI64 and AI64LL cards by checking for a "conversion counter" feature present on the AI64LL.

## Original Source and Documentation

The original source code and documentation for this driver and the associated hardware can be found on the Concurrent Real-Time website.

*   **Website:** [https://www.concurrent-rt.com/](https://www.concurrent-rt.com/)
*   **Product Page (Example):** Search for "16AI64LL" or "PCIe-16AI64SSC-64-50M-0-LL" on their website.

It is recommended to refer to the official documentation for detailed information about the hardware, driver installation, and API usage.

## Maintenance Notes

*   This driver may be a modified version of the original driver from Concurrent Real-Time.
*   When updating or modifying this driver, it is important to consult the original source code and documentation.
*   The source code contains a detailed revision history that can be useful for understanding the changes made to the driver over time.
*   The file `ai64ll/src/ai64ll.c` contains the main driver source code.
*   The file `ai64ll/src/ai64ll_driver.h` contains additional driver definitions.
