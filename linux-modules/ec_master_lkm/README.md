# EtherCAT Master Driver Module

This directory contains the source code for the core EtherCAT master driver module, which is part of the IgH EtherCAT Master stack.

## Module Information

*   **Author:** Florian Pose
*   **Company:** Ingenieurgemeinschaft IgH
*   **Description:** This module is the core of the IgH EtherCAT Master. It implements the EtherCAT master logic, including state machine management, slave device handling, and process data management.
*   **License:** GPLv2

## Functionality

This module works in conjunction with the `ec_generic_lkm` module to provide a complete EtherCAT master solution.

*   **`ec_master_lkm` (this module):** Implements the high-level EtherCAT master protocol and logic.
*   **`ec_generic_lkm`:** Provides the low-level interface to the physical Ethernet hardware.

Together, these modules allow a standard computer running Linux to act as a full-featured EtherCAT master, capable of controlling a network of EtherCAT slave devices.

## Original Source and Documentation

The IgH EtherCAT Master is an open-source project. You can find the source code, documentation, and community support at the following links:

*   **EtherCAT Main Page:** [https://ethercat.org/](https://ethercat.org/)
*   **IgH EtherCAT Master (EtherLab):** [https://etherlab.org/en/ethercat/](https://etherlab.org/en/ethercat/) (Note: This is the historical home of the project, but development has moved to GitLab)
*   **GitLab Repository:** [https://gitlab.com/etherlab.org/ethercat](https://gitlab.com/etherlab.org/ethercat)

## Maintenance Notes

*   This module is the core of the IgH EtherCAT Master stack and depends on the `ec_generic_lkm` module.
*   The main source file for this module is `src/master/module.c`.
*   The `src/master` directory contains the bulk of the EtherCAT master logic.
*   The `src/devices` directory contains header files for the device interface.
