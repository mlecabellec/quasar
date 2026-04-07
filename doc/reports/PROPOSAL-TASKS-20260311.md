# Task Prioritization and Development Roadmap

**Date:** 2026-03-11  
**Project:** Quasar  
**Subject:** Analysis of Open Tasks and Proposed Implementation Strategy

## 1. Executive Summary

A comprehensive review of the `doc/project/tasks` directory identified **15 formal task definitions** (including TSK-20260310-001 and TSK-20260328-001 added after the initial report). As of **2026-04-03**: **2 Completed**, **4 In Progress**, and **9 Not Started**. This report proposes a **5-Phase Roadmap** designed to resolve technical dependencies early and build momentum toward complex UI and logic systems.

## 2. Inventory of Tasks

| Task ID | Goal | Estimated Complexity | Core Dependencies | Status |
| :--- | :--- | :--- | :--- | :--- |
| **TSK-20260311-011** | Lua Engine Validation | Medium-High | None (Core) | 🔄 In Progress |
| **TSK-20260311-001** | Advanced Tree Transformations | Medium-High | `named` library | 🔄 In Progress |
| **TSK-20260311-010** | Calibration Framework | Medium | `named`, `coretypes` | 🔄 In Progress |
| **TSK-20260311-002** | Networking Server (Lua) | Medium | `asio`, `CppServer` | 🔲 Not Started |
| **TSK-20260311-003** | Networking Client (Lua) | Medium | `asio`, `CppServer` | 🔲 Not Started |
| **TSK-20260311-004** | ZeroMQ Plugin | Medium | `named`, `libzmq` | 🔄 In Progress |
| **TSK-20260303-001** | Data Logging / Acquisition | High | `named`, `coretypes` | 🔲 Not Started |
| **TSK-20260311-006** | CAN Bus (SocketCAN) | High | `datacodec`, `named` | 🔲 Not Started |
| **TSK-20260311-005** | OPC UA Server/Client | High | `named`, `networking` | 🔲 Not Started |
| **TSK-20260311-009** | LogicEngine (FSM/SFC) | High | `named`, `scheduler` | ✅ Completed |
| **TSK-20260311-008** | Web UI & REST/WS API | High | `named`, `networking` | 🔲 Not Started |
| **TSK-20260311-007** | Apache Kafka Integration | High | `named`, `librdkafka` | 🔲 Not Started |
| **TSK-20260303-004** | Python Bindings | High | `coretypes`, `named` | 🔲 Not Started |
| **TSK-20260310-001** | Standalone Script Runner (`sre`) | Medium | `scripting`, `named` | ✅ Completed |
| **TSK-20260328-001** | Reflexive Execution & Service Orchestration | High | `named`, `scripting` | 🔲 Not Started |

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
