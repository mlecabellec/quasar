# National Instruments PCIe-6509 Linux Kernel Module (simusol)

This directory contains a Linux kernel module for the National Instruments PCIe-6509 digital I/O card, likely developed for a specific project or simulation environment ("simusol").

## Module Information

*   **Hardware:** National Instruments PCIe-6509
*   **Author:** ************ Anonymized
*   **Company:** ************ (now part of Airbus Defence and Space)
*   **Description:** This is a third-party Linux driver for the National Instruments PCIe-6509, a 96-channel, parallel digital I/O card. The comments in the source code are primarily in French.

## Hardware Overview

The National Instruments PCIe-6509 is a high-density digital I/O card for PCIe-based systems. Key features include:

*   **96 Channels:** 96 bidirectional digital I/O lines.
*   **TTL/CMOS Compatibility:** Compatible with 5V TTL/CMOS logic levels.
*   **Programmable:** Each channel can be individually configured as an input or output.

## Relationship to other Modules

This is the second driver for the NI PCIe-6509 in this repository. The other is `pcie6509`, which was developed by Concurrent Computer Corporation. This "simusol" version appears to be a separate, independent implementation from Astrium.

## Original Source and Documentation

The source code for this driver is located in the `src` directory. For official documentation on the hardware, refer to the National Instruments website.

*   **National Instruments Website:** [https://www.ni.com/](https://www.ni.com/)
*   **Product Page:** [https://www.ni.com/en-us/support/model.pcie-6509.html](https://www.ni.com/en-us/support/model.pcie-6509.html)

## Maintenance Notes

*   This driver is a separate implementation from the one in the `pcie6509` directory.
*   The main initialization logic is in `pcie6509init.c`.
*   The `pcie6509ioctl.c` file implements the `ioctl` interface for user-space communication.
*   The comments in the source code are in French.
