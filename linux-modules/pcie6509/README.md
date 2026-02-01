# National Instruments PCIe-6509 Linux Kernel Module

This directory contains a Linux kernel module for the National Instruments PCIe-6509 digital I/O card.

## Module Information

*   **Hardware:** National Instruments PCIe-6509
*   **Author:** D. Dubash
*   **Copyright:** Concurrent Computer Corporation (2010 and beyond)
*   **Description:** This is a third-party Linux driver for the National Instruments PCIe-6509, a 96-channel, parallel digital I/O card.

## Hardware Overview

The National Instruments PCIe-6509 is a high-density digital I/O card for PCIe-based systems. Key features include:

*   **96 Channels:** 96 bidirectional digital I/O lines.
*   **TTL/CMOS Compatibility:** Compatible with 5V TTL/CMOS logic levels.
*   **Programmable:** Each channel can be individually configured as an input or output.
*   **Watchdog:** Includes a digital I/O watchdog for fault detection and recovery.
*   **Input Filters:** Programmable input filters for debouncing and noise reduction.

## Original Source and Documentation

The source code for this driver is located in the `src` directory. While this is a third-party driver, you can find official documentation for the hardware on the National Instruments website.

*   **National Instruments Website:** [https://www.ni.com/](https://www.ni.com/)
*   **Product Page:** [https://www.ni.com/en-us/support/model.pcie-6509.html](https://www.ni.com/en-us/support/model.pcie-6509.html)

## Maintenance Notes

*   This driver is developed by Concurrent Computer Corporation, not National Instruments.
*   The main driver logic is in `pcie6509.c`.
*   The `pcie6509_user.h` header file defines the ioctl interface for user-space communication.
*   The driver is split into several files, with each file handling a specific aspect of the card's functionality (e.g., `pcie6509_din.c` for digital input, `pcie6509_dout.c` for digital output).
