# Quasar Strategic Roadmap: Q2 2026 Update

**Date:** 2026-04-10  
**Status:** Strategic Alignment & Execution Phase

## 1. Executive Implementation Summary
The Quasar project has successfully transitioned from foundational library development to **industrial connectivity and high-level orchestration**. As of April 10, 2026, the module status is as follows:

| Category | Status | Count | Key Modules |
| :--- | :--- | :--- | :--- |
| ✅ **Completed** | Production-Ready | 8 | `logicengine` (HSM/SFC), `sre` (Script Runner), Core Types, Deep Copy, Reflexive Interface (`ActiveEntity`), Global Logging. |
| 🔄 **In Progress** | Active Dev | 6 | **OPC UA Integration**, **Tree Transformations**, **Data Logging**, **ZeroMQ PUB/SUB**, **Calibration Framework**, **Lua Stability**. |
| 🔲 **Not Started** | Backlog | 7 | Web UI, CAN Bus (SocketCAN), Apache Kafka, Python Bindings, Networking (TCP/UDP/HTTP), Service Orchestration. |

---

## 2. Technical Health & Stability Baseline

### 2.1 Lua Execution Environment
- **Status:** ✅ **Verified Stable** (per `LUA-STABILITY-VALIDATION-20260409.md`).
- **Key Fixes:** Resolved initialization race conditions, implemented cooperative lock yielding for blocking calls, and established strict `ObjectTracker` proxy patterns.

### 2.2 OPC UA Industrial Protocol
- **Status:** 🔄 **Active Troubleshooting** (per `opcua-progress-report-2026-04-07.md`).
- **Current Blocker:** Recursive mirroring logic is failing to discover Methods and Variables in Namespace 1 (NS1), limiting reflexive control over the network.

### 2.3 Standards Compliance (CS-0010)
- **Baseline:** High adherence in core modules (`named`, `coretypes`, `logicengine`).
- **Corrective Actions:** Refactoring `opcua` to eliminate C-style casts and `sim-test` to remove raw `new` calls is required for full architectural integrity.

---

## 3. Short-Term Roadmap (Q2 2026)

### Phase A: Finalizing Industrial Data Pipelines (April - May)
*   **Tree Transformation Engine (TSK-20260311-001):** Finalize the XSLT-inspired reinterpretation engine to enable zero-copy mapping of raw buffers into structured OOP hierarchies.
*   **Calibration Framework (TSK-20260311-010):** Deliver polynomial and point-pair interpolation for engineering unit conversion.
*   **Data Acquisition (TSK-20260303-001):** Validate the high-performance CSV/Binary recorder at 50,000 samples/sec sustained load.

### Phase B: Connectivity & Protocol Stabilization (May - June)
*   **OPC UA Hardening (TSK-20260311-005):** Resolve NS1 discovery blockers and implement UADP (UDP) transport for PubSub.
*   **ZeroMQ Plugin (TSK-20260311-004):** Complete the Tree Serialization bridge for low-latency telemetry distribution.
*   **Networking Foundations (TSK-20260311-002/003):** Establish the asynchronous TCP/UDP/HTTP client/server plugins for the Lua environment.

### Phase C: Orchestration & Visibility (June - July)
*   **Reflexive Orchestration (TSK-20260328-001):** Implement `NamedMethod` and `NamedService` to allow system behavior to be discoverable and executable via protocols.
*   **Web Dashboard (TSK-20260311-008):** Begin development of the React-based "Live View" for real-time system monitoring and control.

---

## 4. Critical Risks & Mitigation Strategies

| Risk | Mitigation Strategy |
| :--- | :--- |
| **OPC UA Mirroring Failure** | Prioritize deep-dive GDB analysis of reference filtering in the `browse` request. |
| **Memory Leaks in Plugins** | Enforce mandatory `LuaProxy<T>` usage and `ObjectTracker` validation for all new bindings. |
| **Concurrency Jitter** | Mandatory **TSan** (ThreadSanitizer) runs for all modules introducing background service threads. |

---
*Roadmap approved and maintained by Quasar Engineering.*
