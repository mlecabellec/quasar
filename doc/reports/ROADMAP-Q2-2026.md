# Quasar Strategic Roadmap: Q2 2026 Update

**Date:** 2026-04-10  
**Status:** Strategic Expansion & Industrial Hardening

## 1. Executive Implementation Summary
The Quasar project has expanded its scope to include **professional-grade EtherCAT engineering**. We are transitioning from simple driver implementation to a reflexive, redundant, and user-friendly industrial master suite.

| Category | Status | Count | Key Modules |
| :--- | :--- | :--- | :--- |
| ✅ **Completed** | Production-Ready | 8 | `logicengine`, `sre`, Core Types, Deep Copy, `ActiveEntity`, Global Logging. |
| 🔄 **In Progress** | Active Dev | 6 | **OPC UA**, **Tree Transformations**, **Data Logging**, **ZeroMQ**, **Calibration**, **Lua Stability**. |
| 🔲 **Not Started** | Backlog/New | 12 | Web UI, CAN Bus, Kafka, Python Bindings, **Reflexive EtherCAT**, **ENI Engineering**, **ecat-top TUI**, **Passive Master**. |

---

## 2. Technical Health & Stability Baseline

### 2.1 Lua Execution Environment
- **Status:** ✅ **Verified Stable**.
- **Focus:** Maintaining strict `ObjectTracker` patterns as we introduce complex EtherCAT and ZMQ bindings.

### 2.2 EtherCAT Master (`resoem`)
- **Status:** 🔄 **Strategic Enhancement Phase**.
- **New Direction:** Decoupling diagnostic sweeps from cyclic exchange, implementing identity-based topology matching, and providing a modern TUI (`ecat-top`).

---

## 3. Short-Term Roadmap (Q2 - Q3 2026)

### Phase A: Industrial Data Pipelines (April - May)
*   **Tree Transformation Engine (TSK-20260311-001):** Finalize zero-copy reinterpretation.
*   **Calibration Framework (TSK-20260311-010):** Deliver engineering unit conversion.
*   **Data Acquisition (TSK-20260303-001):** Validate 50kHz logging.

### Phase B: Connectivity & Protocol Stabilization (May - June)
*   **OPC UA Hardening (TSK-20260311-005):** Resolve NS1 discovery and implement UDP transport.
*   **Reflexive EtherCAT (TSK-20260410-001):** Implement `EthercatMasterService` with reflexive methods and decoupled diagnostics.
*   **Networking Foundations (TSK-20260311-002/003):** Establish async TCP/UDP/HTTP plugins.

### Phase C: Orchestration & Visibility (June - July)
*   **Reflexive Orchestration (TSK-20260328-001):** Implement `NamedMethod` and `NamedService` logic.
*   **Interactive TUI (TSK-20260410-004):** Deliver `ecat-top` for real-time bus orchestration over SSH.
*   **Web Dashboard (TSK-20260311-008):** Begin React-based "Live View" development.

### Phase D: Advanced Industrial Engineering (July - Sept)
*   **Advanced Topology (TSK-20260410-002):** Implement ENI (ETG.2100) generation and Hot Connect support.
*   **Master Redundancy (TSK-20260410-005):** Implement Passive Master and Redundancy Ring failover.
*   **ESI Cache (TSK-20260410-003):** Index vendor SDO sequences for automated configuration.

---

## 4. Critical Risks & Mitigation Strategies

| Risk | Mitigation Strategy |
| :--- | :--- |
| **Topology Mismatch** | Implement identity-tuple matching `(Vendor, Product, Revision, Position)` in `ecat-top`. |
| **Failover Jitter** | Utilize host OS clock synchronization and target < 2 cycle takeover in `RedundantEthercatMaster`. |
| **OPC UA Mirroring** | Prioritize GDB analysis of Namespace 1 reference filtering. |

---
*Roadmap approved and maintained by Quasar Engineering.*
