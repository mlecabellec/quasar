# GSC 16AO Linux Kernel Module

This directory contains the Linux kernel module for the General Standards Corporation (GSC) 16AO family of analog output cards.

## Module Information

*   **Product Family:** 16AO
*   **Manufacturer:** General Standards Corporation
*   **Original Author:** E. Hillman
*   **Copyright:** General Standards Corporation (2002), Concurrent Computer Corporation (2003)
*   **Description:** This is a Linux host driver for the GSC 16AO family of analog output cards, including the PCI/PMC-16AO-12 and GSC16AO16.

## Hardware Overview

The GSC 16AO series are high-speed, high-resolution analog output cards for PCI/PMC based systems. Key features include:

*   **Channels:** Available in 2, 12, or 16 channel configurations.
*   **Resolution:** 16-bit D/A converters.
*   **Output Ranges:** Software-selectable voltage ranges.
*   **Data Rates:** High-speed data rates with a large FIFO buffer.
*   **Autocalibration:** On-demand autocalibration for high accuracy.

These cards are suitable for a variety of applications, including waveform synthesis, process control, and industrial robotics.

## Original Source and Documentation

The source code for this driver is located in the `src` directory. For more information about the hardware, refer to the General Standards Corporation website:

*   **Website:** [https://www.generalstandards.com/](https://www.generalstandards.com/)
*   **Product Page (Example):** [https://www.generalstandards.com/product/16ao16-16-channel-16-bit-differential-analog-output-board/](https://www.generalstandards.com/product/16ao16-16-channel-16-bit-differential-analog-output-board/)

## Maintenance Notes

*   The main driver logic is in `gsc16ao_main.c`.
*   The `gsc16ao_ioctl.h` header file defines the ioctl commands for user-space communication.
*   The `gsc16ao_regs.h` header file defines the hardware registers.
*   The `unused` directory contains older or alternative source files that are not currently part of the build.
