# CEVT Linux Kernel Module

This directory contains the source code for the "CEVT" Linux kernel module.

## Module Information

*   **Original Author:** Y. Guillemot
*   **Description:** This is a "pseudo-driver" called "Concentrateur d'EVenemenTs" (Event Concentrator). It is not a driver for a specific piece of hardware. Instead, it acts as a software layer that works in conjunction with other hardware drivers, such as `asdc_lkm`.
*   **Language:** The source code comments are primarily in French.

## Functionality

The purpose of this pseudo-driver is to aggregate events from various hardware drivers (like the `asdc_lkm` for MIL-STD-1553 cards) and provide a single, unified interface for user-space applications to consume these events. This simplifies the development of applications that need to react to events from multiple hardware sources.

## Relationship with other Modules

The `cevt_lkm` is tightly coupled with the `asdc_lkm` driver. The `asdc_lkm` driver (for SBS Technologies MIL-STD-1553 cards) uses the `cevt_lkm` to signal events to user-space. The comments also mention another driver, "ETOR", which likely also uses this event concentrator.

## Maintenance Notes

*   This module is a software-only "pseudo-driver" and does not interface directly with any hardware.
*   It is a dependency for other drivers, such as `asdc_lkm`.
*   The main source file is `src/cevt.c`.
*   The header file `src/cevtctl.h` defines the ioctl commands and data structures used by user-space applications to interact with the event concentrator.
