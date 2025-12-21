# TSyncPCI API Patches

This directory contains patches for the user-space API of the TSyncPCI driver.

## Contents

This directory does not contain a full API or kernel module. Instead, it holds patches for the header files of the TSyncPCI driver, which is located in the `tsyncpci_lkm` directory.

The patches are for the following files:

*   `tsync.h`
*   `tsync_nonkts.h`

These headers likely define the user-space interface to the `tsyncpci_lkm` kernel module.

## Purpose

The purpose of these patches is to modify the TSyncPCI API, likely to add new features, fix bugs, or adapt it for a specific use case.

## Maintenance Notes

*   To understand the context of these patches, you must examine the source code of the `tsyncpci_lkm` driver.
*   The `tsyncpci_lkm` directory contains the full source code for the driver and its API.
