# GSC 16AI API

This directory contains a user-space API and test suite for the General Standards Corporation (GSC) 16AI family of data acquisition cards, specifically the 16AI64SSA model.

## API Information

*   **Copyright:** ArianeGroup (2020)
*   **Description:** This is a C/C++ API for controlling the GSC 16AI64SSA data acquisition card from user-space.
*   **Dependencies:** The API and test code rely on a utility library called "mastol," which appears to be from ArianeGroup.

## Functionality

The API provides a set of functions for interacting with the `gsc16ai` kernel module. These functions allow a user-space application to:

*   Initialize and open the device
*   Configure the card's parameters (e.g., voltage range, sampling rate)
*   Read data from the card using DMA
*   Send `ioctl` commands to the driver for low-level control

## Hardware

This API is designed to be used with the General Standards Corporation 16AI64SSA data acquisition card. This card is supported by the `gsc16ai` kernel module, which must be loaded for this API to function.

## Source Code

*   **Headers:** The public API is likely defined in `drast_lkm/gsc16ai_api/16ai64ssa.h` and `drast_lkm/gsc16ai_api/16ai64ssa_utils.h`. (Note: these files are not present in this directory, but are included by the test code).
*   **Test Code:** The `test` directory contains a CppUnit test suite that demonstrates how to use the API. `TestGsc16Ai.cpp` is the main test file.
*   **Patches:** The `patches` directory contains a patch for `os_util_thread.c`, which is likely part of the "mastol" utility library.

## Maintenance Notes

*   This is not a kernel module, but a user-space library that depends on the `gsc16ai` kernel module.
*   The API is used to control the GSC 16AI64SSA card.
*   The test code provides a good example of how to use the API.
