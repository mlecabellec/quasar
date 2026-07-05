# EtherCAT Knowledge: SOEM, IgH EtherCAT, and SOES

This document synthesizes knowledge derived from the `linux-modules` directory, specifically focusing on `SOEM`, `ethercat` (IgH), and `SOES`. It also touches upon the internal `resoem` project which aims to modernize the EtherCAT master implementation.

## 1. SOEM (Simple Open EtherCAT Master)

**Location:** `linux-modules/SOEM`

### Overview
SOEM is a user-space library for developing EtherCAT MainDevices (Masters). It is designed for embedded systems and is OS-aware but can run on Linux.

*   **Type:** Master (MainDevice) Stack
*   **License:** GPLv3 + Commercial (Dual License)
*   **Language:** C
*   **Architecture:** User-space library. Does not require kernel modules (uses raw sockets).

### Features
*   Services: CoE (CANopen over EtherCAT), SoE (Servo over EtherCAT), FoE (File over EtherCAT).
*   Configuration: Dynamic configuration, generic slave handling.
*   Portability: OS Abstraction Layer (OSAL) and Hardware Abstraction Layer (OSHW).

### Build System
*   **System:** CMake
*   **Targets:** `soem` (static library by default), `soemConfig` (CMake package).
*   **Options:** `EC_DEBUG` (Debug info), `BUILD_SHARED_LIBS`.

### Key Files
*   `src/ec_main.c`: Core state machine and main loop.
*   `osal/linux/osal.c`: Linux-specific OS abstraction (timers, threads, mutexes).
*   `oshw/linux/nicdrv.c`: Network driver interface (using raw sockets).
*   `include/soem/ethercattype.h`: EtherCAT protocol definitions.

---

## 2. IgH EtherCAT Master (linux-modules/ethercat)

**Location:** `linux-modules/ethercat`

### Overview
The IgH EtherCAT Master is a kernel-space implementation. It is known for high performance and real-time capabilities as it runs directly in the kernel, bypassing user-space switching overheads for packet processing.

*   **Type:** Master (MainDevice) Stack
*   **License:** GPLv2
*   **Language:** C/Kernel C
*   **Architecture:** Kernel Modules (`ec_master.ko`, generic or specialized network drivers).

### Features
*   **Real-time:** Hard real-time support with RTAI, Xenomai, or RT-PREEMPT.
*   **Device Drivers:** specialized drivers for many NICs (e.g., `ec_8139too`, `ec_e1000`) for zero-copy operation.
*   **Userspace API:** Allows user-space applications to interact with the kernel master via `ioctl` (using `libethercat`).

### Build System
*   **System:** Autotools (configure/make) wrapped in CMake.
*   **CMake Wrapper:** The `CMakeLists.txt` in this directory is a custom wrapper that calls `./bootstrap`, `./configure`, and `make modules`.
*   **DKMS:** Includes a `dkms.conf` and a `ethercat_dkms` CMake target to install the kernel modules via Dynamic Kernel Module Support (DKMS). having this integrated into CMake facilitates system deployment.

### Key Files
*   `master/`: Core master code.
*   `devices/`: Network device drivers.
*   `lib/`: User-space library source (`libethercat`).
*   `dkms.conf.in`: Template for DKMS configuration.

---

## 3. SOES (Simple Open EtherCAT Slave)

**Location:** `linux-modules/SOES`

### Overview
SOES is a library for implementing EtherCAT Slave devices (SubDevices). It is the counterpart to SOEM.

*   **Type:** Slave (SubDevice) Stack
*   **License:** GPL (Modified, check specific files)
*   **Language:** C
*   **Architecture:** Firmware library for microcontrollers (or Linux for testing).

### Features
*   Implements the EtherCAT Slave Controller (ESC) logic in software (or interfaces with hardware ESC).
*   Support for CoE, FoE.
*    Portable C code.

### Build System
*   **System:** CMake
*   **Targets:** `soes` library.

### Key Files
*   `soes/esc.c`: EtherCAT Slave Controller emulation/interface.
*   `soes/esc_coe.c`: CANopen over EtherCAT implementation.

---

## 4. Internal Project: Resoem

**Location:** `cmake-projects/resoem`

### Overview
`resoem` appears to be a **reimplementation** of SOEM using modern C++23.

*   **Goal:** Create a clean-room implementation to move away from SOEM's GPLv3 license to a permissive one (Internal/Apache 2.0).
*   **Tech Stack:** C++23.
*   **Current State:**
    *   Implements `RawSocket` for Linux.
    *   Basic `EtherCATFrame` handling.
    *   `Enumerator` logic.

### Relationship to Modules
The `linux-modules/SOEM` directory likely serves as a reference implementation for `resoem` and as a fallback/legacy master until `resoem` is feature-complete.
