# Digicool Linux Kernel Module (PCIG)

This directory contains the source code for the "digicool" Linux kernel module, which is built upon a generic driver framework called "PCIG".

## Module Information

*   **Author:** Frédéric Deville
*   **Email:** frederic.deville@adas.fr
*   **Description:** This module is an instance of a generic PCI/PCIe driver framework ("Driver générique PCIG"). The specific device this module is intended for is referred to as "digicool".
*   **Company (likely):** The author's email domain suggests the company is ADAS.

## Hardware

This driver is designed for a line of PCI/PCIe cards that use PLX Technology bridge chips (such as the 9080, 9054, 9056, and 8311). The "digicool" is likely a specific product from ADAS that utilizes this generic driver.

The driver uses the following PLX Vendor and Device IDs:

*   **Vendor ID:** 0x10b5 (PLX Technology)
*   **Device IDs:** 0x9080, 0x9054, 0x9056
*   **Subsystem Device ID (for PCIe):** 0x8311

## Functionality

The driver appears to be a skeleton or framework that provides a generic way to interact with a family of PCI/PCIe cards. It includes functionality for:

*   Memory mapping (PCI, PLX, and device-specific registers)
*   DMA (Direct Memory Access)
*   Interrupt handling
*   EEPROM reading/writing
*   FPGA configuration

## Original Source and Documentation

Due to the generic nature of the driver and the lack of specific product information for "digicool", it is difficult to locate original documentation. The author's email suggests that the original source and documentation may be internal to the company "ADAS".

Future maintainers should start by investigating the hardware and any available documentation from ADAS.

## Maintenance Notes

*   The driver is split into several files, with `pcigSkel.c` providing the main skeleton and `digicool` being a specific implementation for the "digicool" device.
*   `pcigInternals.h` contains the PCI vendor and device IDs.
*   `pcigdrv.h` defines the ioctl interface used to communicate with the driver from user-space.
*   The name "digicool" is used in the configuration and init scripts, confirming it as the name for this specific module instance.
