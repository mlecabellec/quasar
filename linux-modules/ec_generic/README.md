# EtherCAT Generic Ethernet Device Module

This directory contains the source code for the EtherCAT generic Ethernet device module, which is part of the IgH EtherCAT Master stack.

## Module Information

*   **Author:** Florian Pose
*   **Company:** Ingenieurgemeinschaft IgH
*   **Description:** This module acts as a generic interface between the IgH EtherCAT Master and standard Ethernet devices.
*   **License:** GPLv2

## Functionality

EtherCAT is a high-performance, real-time Ethernet protocol used in industrial automation. The IgH EtherCAT Master is an open-source master stack for Linux that allows a standard computer to act as an EtherCAT master.

This "ec_generic" module is a key component of the IgH EtherCAT Master. It works by creating a raw network socket and binding it to a physical Ethernet interface. This allows the EtherCAT master stack to bypass the kernel's normal networking stack and send and receive EtherCAT frames directly, which is essential for achieving the low latency and real-time performance required by EtherCAT.

This module is not a driver for a specific piece of hardware. Instead, it's a software-only driver that can work with any standard Ethernet card supported by Linux.

## Original Source and Documentation

The IgH EtherCAT Master is an open-source project. You can find the source code, documentation, and community support at the following links:

*   **EtherCAT Main Page:** [https://ethercat.org/](https://ethercat.org/)
*   **IgH EtherCAT Master (EtherLab):** [https://etherlab.org/en/ethercat/](https://etherlab.org/en/ethercat/) (Note: This is the historical home of the project, but development has moved to GitLab)
*   **GitLab Repository:** [https://gitlab.com/etherlab.org/ethercat](https://gitlab.com/etherlab.org/ethercat)

## Maintenance Notes

*   This module is a core part of the IgH EtherCAT Master stack.
*   It is not a standalone driver and requires the other components of the EtherCAT master to be installed.
*   The main source file is `src/generic.c`.
*   The header file `src/ecdev.h` defines the interface between this module and the rest of the EtherCAT master stack.
