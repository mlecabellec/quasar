# UCEI PCI API

This directory contains a user-space API and test suite for the Abaco Systems (formerly GE Intelligent Platforms) RPCIe1553 MIL-STD-1553 interface card.

## API Information

*   **Hardware:** Abaco Systems RPCIe1553
*   **Description:** This is a C++ API for controlling the RPCIe1553 card from user-space. The name "UCEI" is used in the test code and is likely an internal or project-specific name for this API.
*   **Dependencies:** The test code uses the CppUnit testing framework.

## Functionality

The API provides a set of classes and functions for interacting with the RPCIe1553 card. Based on the test code, the API supports:

*   Finding and opening the device.
*   Configuring the card for Bus Controller (BC) and Remote Terminal (RT) operation.
*   Starting and stopping communication on the 1553 bus.
*   Injecting and detecting errors.
*   Querying board information, such as serial number and API version.

The test code also mentions "BusTools", which is a comprehensive software suite from Abaco Systems for MIL-STD-1553 test, analysis, and simulation. This API is likely a C++ interface to the low-level drivers that are part of the BusTools suite.

## Hardware Overview

The Abaco Systems RPCIe1553 is a high-density PCI Express interface card for MIL-STD-1553 applications. It is available with one, two, or four dual-redundant channels and can be configured to operate as a Bus Controller, multiple Remote Terminals, and a Bus Monitor.

## Original Source and Documentation

The source code for this API is not fully contained in this directory. The `patches` directory contains patches for a number of header files (like `btdrv.h`), which suggests that this API is a customization of a more complete API from Abaco Systems.

For more information about the hardware and the BusTools software, refer to the Abaco Systems website:

*   **Website:** [https://www.abaco.com/](https://www.abaco.com/)
*   **Product Page:** [https://www.abaco.com/products/rpci-1553](https://www.abaco.com/products/rpci-1553)

## Maintenance Notes

*   This is a user-space API, not a kernel module.
*   The `uceipci_lkm` directory likely contains the corresponding kernel module for this API.
*   The `test` directory contains a CppUnit test suite that demonstrates how to use the API.
*   The `patches` directory contains modifications to the original API headers, which may provide insight into the specific customizations made for this project.
