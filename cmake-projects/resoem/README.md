# Resoem

A clean-room, C++23 reimplementation of the Simple Open EtherCAT Master (SOEM) library.

## Goals

1.  **License Freedom**: Replace the GPLv3/Commercial dual-license of the original SOEM with a permissive license (internal/Apache 2.0).
2.  **Modernization**: Use modern C++23 features for better safety, readability, and performance.
3.  **Integration**: Seamlessly integrate with the `quasar` project structure.

## Architecture

-   **Transport**: Raw Ethernet socket wrapper (`PF_PACKET`) for Linux.
-   **Core**: Object-oriented representation of the EtherCAT master and slaves.
-   **API**: Type-safe headers for CoE (CANopen over EtherCAT) and SoE (Servo over EtherCAT).
