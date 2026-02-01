# Acromag VFX70 Linux Kernel Module

This directory contains the Linux kernel module for the Acromag PMC-VFX70, a user-configurable FPGA module.

## Module Information

*   **Product:** PMC-VFX70
*   **Manufacturer:** Acromag
*   **Authors:** FJM (Acromag), Louis Herve (Astrium), ************ Anonymized (Astrium)
*   **Description:** This is a Linux driver for the Acromag PMC-VFX70, a PMC card with a Xilinx Virtex-5 FPGA.

## Hardware Overview

The Acromag PMC-VFX70 is a high-performance PMC module designed for embedded computing applications. Its key feature is a user-reconfigurable Xilinx Virtex-5 FPGA, which allows for the implementation of custom logic and processing pipelines.

*   **FPGA:** Xilinx Virtex-5 (XC5VFX70T) with an embedded PowerPC 440 processor.
*   **Interface:** 64-bit, 100MHz PCI-X bus.
*   **I/O:** 64 I/O lines or 32 LVDS lines, with support for front and rear I/O.
*   **Memory:** Multiple high-speed memory buffers.

## Relationship to other Modules

This driver is a dependency for the `eabi` module. The `eabi` uses the VFX70 hardware and this driver to emulate a MIL-STD-1553 interface.

## Original Source and Documentation

The source code for this driver is located in the `src` directory. For more information about the hardware, refer to the Acromag website:

*   **Website:** [https://www.acromag.com/](https://www.acromag.com/)
*   **Product Page:** [https://www.acromag.com/products/io-cards/pmc-modules/pmc-vfx70-virtex-5-fpga-modules-pci-x/](https://www.acromag.com/products/io-cards/pmc-modules/pmc-vfx70-virtex-5-fpga-modules-pci-x/)

## Maintenance Notes

*   The main driver logic is in `devvfx.c`.
*   `vfx_ctl.h` contains important definitions, including the device ID (`PMCVFXBOARD`).
*   The driver is designed to be used in conjunction with other modules, such as `eabi`, which provides higher-level functionality.
*   The authors of this driver have affiliations with both Acromag and Astrium (now part of Airbus Defence and Space), suggesting a collaborative development effort.
