# EtherCAT Knowledge: IgH EtherCAT, Resoem, and SOES

This document synthesizes knowledge regarding EtherCAT implementations in Quasar: `ethercat` (IgH kernel master in `linux-modules/ethercat`), the modernized C++23 master implementation `resoem` (`cmake-projects/resoem`), and `SOES` (`cmake-projects/third-party/SOES`). The legacy C `SOEM` and stub `SOES` directories previously located under `linux-modules` have been removed as `resoem` fully supersedes `SOEM` and slave support is consolidated in `cmake-projects/third-party/SOES`.

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

## 4. Internal Projects: `resoem` and `resoes`

**Locations:** `cmake-projects/resoem` and `cmake-projects/resoes`

### Overview (`resoem` MainDevice)
`resoem` is a clean-room C++23 EtherCAT MainDevice (Master) stack moving away from GPLv3 C dependencies to Quasar's internal safety and licensing standards.

*   **Tech Stack:** C++23 (`CS-0010`..`CS-0090`, `std::timed_mutex`, `std::expected`).
*   **Current Capabilities:**
    *   `RawSocket` / `IRawSocket` transport (`AF_PACKET`) and `EtherCATFrame` multi-datagram builder/parser.
    *   `Enumerator`, `SlaveInfo`, `ProcessImage`, Distributed Clocks (`dc_calc`), and `Diagnostics`.
    *   Mailbox protocols: `MailboxHandler`, `CoEHandler`, `FoEHandler`, `EoEHandler`, `SoEHandler`.
    *   ETG.2100 ENI engineering: `EniDocument`, `EniConfigurator`, `EniGenerator`, `EniComparator` (`TSK-20260920-001`).
    *   Resilience & hardware-free CI simulation: `HotConnectManager`, `RedundancyManager`, `VirtualEtherCATBus`, `SimulatedSlave`, `StubRawSocket`, and `EthercatMasterService` (`ecat-top` TUI).

### Overview (`resoes` SubDevice)
`resoes` (`TSK-20261113-001`) is the clean-room C++23 EtherCAT SubDevice (Slave) counterpart to `SOES`:

*   **Zero Globals / Explicit Error Handling:** Context-owned classes returning `std::expected<T, SlvError>` without C unions or `auto`.
*   **Core Stack:** `EscController` (ESM, SM0-SM3, Watchdog, DC checks), `MailboxServer` (`SM0`/`SM1` pool & protocol multiplexer), `ObjectDictionary`, `CoeServer` (Expedited/Segmented/Complete-Access SDO, SDO Info, `0x1C12`/`0x1C13` sizing), `EepromEmulator` (`4B`/`8B` SII), `IEscHardware` / `MemoryEscHardware`, and `SlaveApplication` (`DIG_process` / `step_freerun`).

### Relationship to Modules
`resoem` is the primary C++23 user-space master stack and `resoes` is the companion C++23 slave stack for Quasar. The legacy `linux-modules/SOEM` directory has been removed. See [Resoem Classes](../impl/classes/resoem/README.md) and [Resoes Classes](../impl/classes/resoes/README.md).
