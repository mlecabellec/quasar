# APMC424 Linux Kernel Module

This directory contains the Linux kernel module for the Acromag PMC424 Digital I/O and Counter/Timer module.

## Module Information

*   **Vendor:** Acromag
*   **Vendor ID:** 0x16D5
*   **Device:** PMC424
*   **Device ID:** 0x4243
*   **Description:** This driver provides an interface to the Acromag PMC424, a PMC module with digital I/O channels and multi-function counter/timers.

## Hardware Overview

The Acromag PMC424 is a PMC module with the following features:

*   **Digital I/O:** 40 channels, including 24 differential I/O and 16 TTL I/O.
*   **Counter/Timers:** Four 16-bit multi-function counter/timers that can be combined into two 32-bit counters.
*   **Interrupts:** Supports interrupts on all input channels based on programmable level transitions or change-of-state.

## Original Source and Documentation

The original source code and documentation for this driver and the associated hardware can be found on the Acromag website.

*   **Website:** [https://www.acromag.com/](https://www.acromag.com/)
*   **Product Page:** [https://www.acromag.com/products/io-cards/pmc-modules/pmc424-40-channel-digital-io-module-four-16-bit-counters/](https://www.acromag.com/products/io-cards/pmc-modules/pmc424-40-channel-digital-io-module-four-16-bit-counters/)

It is recommended to refer to the official documentation for detailed information about the hardware, driver installation, and API usage.

## Maintenance Notes

*   This driver may be a modified version of the original driver from Acromag.
*   The source code is split into two main directories:
    *   `src/pmc424`: Contains the device-specific code for the PMC424.
    *   `src/pmccommon`: Contains common code that may be shared with other Acromag PMC drivers.
*   The main driver source file is `src/pmc424/dev424/dev424.c`.
*   The header file `src/pmccommon/pmcmulticommon.h` contains common definitions, including the Acromag `VENDOR_ID`.
