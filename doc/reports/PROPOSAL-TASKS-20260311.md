# Task Prioritization and Development Roadmap

**Date:** 2026-04-19 (Updated)
**Project:** Quasar  
**Subject:** Analysis of Open Tasks and Proposed Implementation Strategy

## 1. Executive Summary

A comprehensive review of the `doc/project/tasks` directory identified **15 formal task definitions** plus specialized EtherCAT engineering initiatives. As of **2026-04-19**: **8 Completed**, **6 In Progress**, and **12 Not Started (Backlog)**. This report reflects the shift toward industrial-grade fieldbus management and the successful stabilization of the core execution layers.

## 2. Inventory of Tasks

| Task ID | Goal | Core Dependencies | Status |
| :--- | :--- | :--- | :--- |
| **TSK-20260311-011** | Lua Engine Validation | None (Core) | 🔄 In Progress (Refactoring) |
| **TSK-20260311-001** | Advanced Tree Transformations | `named` library | 🔄 In Progress |
| **TSK-20260311-010** | Calibration Framework | `named`, `coretypes` | 🔄 In Progress |
| **TSK-20260311-002** | Networking Server (Lua) | `asio`, `CppServer` | 🔲 Not Started |
| **TSK-20260311-003** | Networking Client (Lua) | `asio`, `CppServer` | 🔲 Not Started |
| **TSK-20260311-004** | ZeroMQ Plugin | `named`, `libzmq` | 🔄 In Progress |
| **TSK-20260303-001** | Data Logging / Acquisition | `named`, `coretypes` | 🔄 In Progress |
| **TSK-20260311-006** | CAN Bus (SocketCAN) | `datacodec`, `named` | 🔲 Not Started |
| **TSK-20260311-005** | OPC UA Server/Client | `named`, `networking` | 🔄 In Progress |
| **TSK-20260311-009** | LogicEngine (FSM/SFC) | `named`, `scheduler` | ✅ Completed |
| **TSK-20260311-008** | Web UI & REST/WS API | `named`, `networking` | 🔲 Not Started |
| **TSK-20260311-007** | Apache Kafka Integration | `named`, `librdkafka` | 🔲 Not Started |
| **TSK-20260303-004** | Python Bindings | `coretypes`, `named` | 🔲 Not Started |
| **TSK-20260310-001** | Standalone Script Runner (`sre`) | `scripting`, `named` | ✅ Completed |
| **TSK-20260328-001** | Reflexive Orchestration | `named`, `scripting` | 🔲 Not Started |
| **TSK-20260410-001** | EtherCAT Master Diag Suite | `resoem`, `named` | 🔲 Not Started |
| **TSK-20260410-004** | `ecat-top` Interactive TUI | `ftxui`, `resoem` | 🔲 Not Started |

## 3. Implementation Roadmap

### Phase 1: Foundations & Stability
**Objective:** Harden the core platform and provide essential library utilities.

1.  **Lua Engine Validation (TSK-20260311-011):** Critical for ensuring that the plugin-based architecture is robust before building complex networking or industrial stacks on top of it.
2.  **Advanced Tree Transformations (TSK-20260311-001):** Adds `map`, `filter`, and `reduce` to the `NamedObject` hierarchy. This is a functional prerequisite for the Web UI (tree synchronization) and the Logic Engine.
3.  **Calibration Framework (TSK-20260311-010):** Provides engineering unit conversions, essential for meaningful data logging and UI visualization.

### Phase 2: Core Connectivity
**Objective:** Enable basic communication patterns.

4.  **Networking Plugins (TSK-20260311-002 / TSK-20260311-003):** Provides the async TCP/UDP/HTTP building blocks used by later phases (Web UI, OPC UA).
5.  **ZeroMQ PUB/SUB (TSK-20260311-004):** Enables high-performance telemetry distribution.

### Phase 3: Industrial Infrastructure
**Objective:** Bridge Quasar to physical processes and persistent storage.

6.  **Data Logging Service (TSK-20260303-001):** Implements the recording backbone for diagnostics.
7.  **CAN Bus / SocketCAN (TSK-20260311-006):** Real-time hardware fieldbus integration.
8.  **OPC UA Integration (TSK-20260311-005):** Standard industrial interoperability.

### Phase 4: High-Level Orchestration
**Objective:** Implement the "Action" layer of the automation system.

9.  **Logic Engine (TSK-20260311-009):** Implements FSM, SFC (Grafcet), and Rule engines for deterministic control.
10. **Web UI & Dashboard (TSK-20260311-008):** Centralized control and visibility. Depends on Networking and Tree Transformations.

### Phase 5: Ecosystem Expansion
**Objective:** Connect to Enterprise and Data Science tools.

11. **Apache Kafka (TSK-20260311-007):** Big data streaming.
12. **Python Bindings (TSK-20260303-004):** Native analysis and data engineering bridge.

## 4. Key Activities & Risks

*   **Concurrency Verification:** Many tasks (ZMQ, Kafka, Networking) introduce background threads. Strict adherence to `CS-0010` and aggressive use of **TSan** is mandatory.
*   **API Mapping Overload:** Creating bindings for Lua/Python for over 10 modules is a large "boilerplate" effort. Automation or macro-based binding generation (e.g., using `sol2` or `nanobind` features) should be investigated early.
*   **Dependency Management:** The move to `FetchContent` for external libraries (`open62541`, `librdkafka`, `nanobind`) must be consistent to avoid build system fragmentation.

---
> [!IMPORTANT]
> This prioritization prioritizes **Stability -> Connectivity -> Control -> Visibility**. It ensures that the "brain" (Logic Engine) and "eyes" (Web UI) are built on top of a verified, high-performance communication and data foundation.

## 5. Module-Specific Questions & Confirmed Baselines

### 5.1 Confirmed Technical Baselines (Lua Implementation)
*   **Concurrency**: **Thread-Per-Script**. Each module operates in total isolation.
*   **Security**: **No Sandboxing**. Scripts have full system/IO access.
*   **Interaction**: **Proxy Design Pattern**. Strict decorrelation between C++ core and Lua engine.
*   **Reactivity**: **Watermarked Async Queues**. Guaranteed delivery up to a configurable memory limit.

### 5.2 Remaining Open Questions
1.  **OPC UA Strategy (TSK-20260311-005):**
    *   *Question*: Is `open62541` the definite choice, or should we evaluate **S2OPC** for security-certified environments?
2.  **Web UI Architecture (TSK-20260311-008):**
    *   *Question*: Should we prioritize a **"No-Build" ES Modules approach** for simplicity, or a **React/Vite** setup for complex dashboards?
3.  **CAN Signal Mapping (TSK-20260311-006):**
    *   *Question*: Does `datacodec` already contain a **DBC/JSON parser**, or is this part of Phase 3 development?
4.  **Kafka Resilience (TSK-20260311-007):**
    *   *Question*: Favor **"At-Most-Once"** (drop data) or **"At-Least-Once"** (heavy buffering) during disconnects?
5.  **Logic Engine Integration (TSK-20260311-009):**
    *   *Question*: Use the **centralized Scheduler** or a dedicated **High-Priority thread** for SFC execution?
