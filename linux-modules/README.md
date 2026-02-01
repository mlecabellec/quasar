# Linux Kernel Modules

This document provides a summary of the Linux kernel modules found in this repository. Each section describes a module, its associated hardware, and its distinctive features.

| Module | Hardware | Features |
|---|---|---|
| `ai64` | General Standards AI64 family (PMC/PCI) | Data acquisition card driver. |
| `ai64ll` | Concurrent Real-Time AI64LL family | Low-latency analog input card driver. |
| `apmc424` | Acromag PMC424 (PMC) | Digital I/O and Counter/Timer module. |
| `asdc` | SBS Technologies MIL-STD-1553 family (PMC) | MIL-STD-1553 interface (BC, RT, BM). |
| `cevt` | N/A (pseudo-driver) | Aggregates hardware events for user-space. |
| `digicool` | ADAS "digicool" cards with PLX bridge chips (PCI/PCIe) | Generic driver framework for PCI/PCIe cards. |
| `dummy` | N/A | Template for creating new kernel modules. |
| `eabi` | Acromag PMC-VFX70 (PMC) | Emulates a MIL-STD-1553 interface using an FPGA. |
| `ec_generic` | Any standard Ethernet card | Generic Ethernet device driver for IgH EtherCAT master. |
| `ec_master` | N/A | IgH EtherCAT master implementation. |
| `gsc16ai` | General Standards 16AI64SSA (PCI/PCIe) | 64-channel simultaneous sampling analog input. |
| `gsc16ao` | General Standards 16AO family (PCI/PMC) | Analog output cards with up to 16 channels. |
| `opto16x16` | General Standards OPTO16X16 (PCIe) | 16 isolated digital inputs and 16 isolated digital outputs. |
| `pcie6509` | National Instruments PCIe-6509 | 96-channel digital I/O card driver (by Concurrent). Includes watchdog and input filters. |
| `pcie6509_simusol`| National Instruments PCIe-6509 | 96-channel digital I/O card driver (by Astrium). |
| `rfm2g` | GE/Abaco RFM2g family (PCI, PMC, PCIe, VME) | 2 Gbit/s Reflective Memory network driver. |
| `tsyncpci` | Spectracom TSync family (PCI, PCIe, cPCI) | High-precision time synchronization (GPS, IRIG, etc.). |
| `uceipci` | Abaco/Condor UCEI family (PCIe) | Universal driver for MIL-STD-1553 and ARINC-429 cards. |
| `vfx70` | Acromag PMC-VFX70 (PMC) | User-configurable FPGA module driver. Used by `eabi`. |

## ai64

*   **Hardware:** General Standards Corporation AI64 family of data acquisition cards (PMC-12AI64, PMC-16AI64, PMC-16AI64SS, PMC-16AI64SSA/C). These cards are based on the PLX Technology PCI9080 and PCI9056 bridge chips.
*   **Features:** Linux host driver for the PMC-16AI64SS card and other related models.

## ai64ll

*   **Hardware:** Concurrent Real-Time AI64LL family of analog input cards (e.g., 16AI64LL). These cards are closely related to the General Standards Corporation AI64 series.
*   **Features:** Linux host driver for the 16AI64LL (16-bit) Analog Input card. The "LL" likely stands for "Low Latency". Differentiates between the AI64 and AI64LL cards by checking for a "conversion counter" feature present on the AI64LL.

## apmc424

*   **Hardware:** Acromag PMC424 Digital I/O and Counter/Timer module (PMC).
*   **Features:** 40 digital I/O channels (24 differential I/O and 16 TTL I/O). Four 16-bit multi-function counter/timers (can be combined into two 32-bit counters). Supports interrupts on all input channels based on programmable level transitions or change-of-state.

## asdc

*   **Hardware:** SBS Technologies (now Abaco Systems) family of MIL-STD-1553 interface cards, such as the ABI (Advanced Bus Interface) and ASF (Advanced Single Function) series (ABI-PMC2, ASF-PMC2).
*   **Features:** Provides an interface to the SBS Technologies MIL-STD-1553 interface cards. Enables a computer to perform as a Bus Controller (BC), up to 31 Remote Terminals (RTs), and a Bus Monitor (BM).

## cevt

*   **Hardware:** None (pseudo-driver).
*   **Features:** "Concentrateur d'EVenemenTs" (Event Concentrator). Software layer that works in conjunction with other hardware drivers, such as `asdc`. Aggregates events from various hardware drivers and provides a single, unified interface for user-space applications to consume these events. Tightly coupled with the `asdc` driver.

## digicool

*   **Hardware:** PCI/PCIe cards that use PLX Technology bridge chips (such as the 9080, 9054, 9056, and 8311). The "digicool" is likely a specific product from ADAS that utilizes this generic driver.
*   **Features:** Instance of a generic PCI/PCIe driver framework ("Driver générique PCIG"). Memory mapping (PCI, PLX, and device-specific registers). DMA (Direct Memory Access). Interrupt handling. EEPROM reading/writing. FPGA configuration.

## dummy

*   **Hardware:** None (not intended for use with any specific hardware).
*   **Features:** Serves as a template for creating new Linux kernel modules. Provides a basic structure for a Linux kernel module, including `init` and `exit` functions, module licensing, and a description. Not a functional driver.

## eabi

*   **Hardware:** Acromag PMC-VFX70 (PMC card with a Xilinx Virtex-5 FPGA).
*   **Features:** "Emulated Advanced Bus Interface". Composite driver that provides MIL-STD-1553 functionality. Uses the Acromag PMC-VFX70 to emulate a dedicated MIL-STD-1553 interface card. Composed of three main components: VFX70 Driver, ASDC Driver, and CEVT Driver.

## ec_generic

*   **Hardware:** Any standard Ethernet card supported by Linux. It's a software-only driver.
*   **Features:** Part of the IgH EtherCAT Master stack. Acts as a generic interface between the IgH EtherCAT Master and standard Ethernet devices. Creates a raw network socket and binds it to a physical Ethernet interface, bypassing the kernel's normal networking stack. Essential for achieving low latency and real-time performance required by EtherCAT.

## ec_master

*   **Hardware:** Not specific hardware, but it allows a standard computer running Linux to act as a full-featured EtherCAT master.
*   **Features:** Part of the IgH EtherCAT Master stack. Implements the high-level EtherCAT master protocol and logic. Works in conjunction with the `ec_generic` module. Allows a standard computer running Linux to act as a full-featured EtherCAT master, capable of controlling a network of EtherCAT slave devices.

## gsc16ai

*   **Hardware:** General Standards Corporation (GSC) 16AI64SSA data acquisition card (PCI/PCIe).
*   **Features:** 16-bit, 64-channel, simultaneous sampling analog input card. Designed for high-performance data acquisition and control applications.

## gsc16ao

*   **Hardware:** General Standards Corporation (GSC) 16AO family of analog output cards (PCI/PMC-16AO-12, GSC16AO16).
*   **Features:** 2, 12, or 16 channel configurations. 16-bit D/A converters. Software-selectable voltage ranges. High-speed data rates with a large FIFO buffer. On-demand autocalibration for high accuracy.

## opto16x16

*   **Hardware:** General Standards Corporation (GSC) OPTO16X16 digital I/O board (PCIe).
*   **Features:** 16 digital inputs and 16 digital outputs, each with 5000V of electrical isolation. Change-of-State Interrupts. Debounce rates, change-of-state detection, and interrupts are software-programmable. Supports DMA bus mastering.

## pcie6509

*   **Hardware:** National Instruments PCIe-6509 digital I/O card.
*   **Features:** Third-party Linux driver (from Concurrent Computer Corporation). 96 bidirectional digital I/O lines. TTL/CMOS compatibility. Each channel can be individually configured as an input or output. Includes a digital I/O watchdog for fault detection and recovery. Programmable input filters for debouncing and noise reduction.

## pcie6509_simusol

*   **Hardware:** National Instruments PCIe-6509 digital I/O card.
*   **Features:** Third-party Linux driver (from Astrium ST). 96 bidirectional digital I/O lines. TTL/CMOS compatibility. Each channel can be individually configured as an input or output. This is a separate, independent implementation from `pcie6509`.

## rfm2g

*   **Hardware:** GE Intelligent Platforms (now Abaco Systems) RFM2g family of Reflective Memory devices (PCI, PMC, PCIe, VME).
*   **Features:** High-speed networking (2 Gbit/s) that allows multiple computers to share a common memory space. Changes to memory on one computer are automatically "reflected" to all other computers on the network.

## tsyncpci

*   **Hardware:** Spectracom TSync family of time synchronization cards (PCI, PCIe, cPCI).
*   **Features:** High-precision time synchronization (nanosecond-level resolution). Synchronized to GPS, IRIG, 1PPS, and other time sources. Holdover capability with an internal oscillator. Multiple time code outputs, programmable "heartbeat" signals, and event timestamping inputs.

## uceipci

*   **Hardware:** Abaco Systems (formerly Condor Engineering and GE Intelligent Platforms) family of avionics interface cards, including the RPCIe1553 (PCI Express interface card for MIL-STD-1553).
*   **Features:** Universal Linux driver for a wide range of Abaco Systems avionics interface cards. Supports MIL-STD-1553 and ARINC-429 products. Works with the BusTools API.

## vfx70

*   **Hardware:** Acromag PMC-VFX70, a user-configurable FPGA module with a Xilinx Virtex-5 FPGA.
*   **Features:** 64-bit, 100MHz PCI-X bus. 64 I/O lines or 32 LVDS lines. Dependency for the `eabi` module to emulate a MIL-STD-1553 interface.
