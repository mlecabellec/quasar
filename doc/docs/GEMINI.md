# 🌌 Quasar: Strategic Knowledge Base & Operational Manual

This document consolidates architectural mandates, feature specifications, and active task tracking.

## 🎯 Project Vision & Source Mapping
Quasar is a deterministic industrial automation framework bridging hardware communication with reflexive logic.

| Module | Purpose | Source Directory |
| :--- | :--- | :--- |
| **`coretypes`** | Safe primitives & math | `cmake-projects/coretypes` |
| **`named`** | Hierarchical Tree & Lifecycle | `cmake-projects/named` |
| **`scripting`** | Lua/Python Execution | `cmake-projects/scripting` |
| **`resoem`** | Modern EtherCAT Master | `cmake-projects/resoem` |
| **`datacodec`** | Binary-to-Tree Mapping | `cmake-projects/datacodec` |
| **`logicengine`** | HSM/SFC Logic Engine | `cmake-projects/logicengine` |
| **`calibration`** | Engineering Unit Conversion | `cmake-projects/calibration` |

---

## 🛠️ Mandatory Technical Standards (CS-0000)
*Violation of these rules constitutes a failure of structural integrity. Full details in `doc/architecture/`.*

### 1. Language & Primitives (CS-0010, CS-0020)
*   **Standards**: C++23 mandatory, C++26 features encouraged where compiler-supported (`CS-0010`, `CS-0020`).
*   **Memory**: No `new`/`delete`/`malloc`. Use Smart Pointers (`shared_ptr`, `unique_ptr`) and RAII for all resources (`CS-0010.6-24`).
*   **Types**: NO `auto`. Explicit type declarations required (`CS-0010.34`). Move semantics mandatory for value passing/return (`CS-0010.2-4`).
*   **Safety**: Mandatory handling of all potentially throwing functions (`CS-0010.20`). Timed mutexes required for all shared modifications (`CS-0010.46`).

### 2. Integrity & Determinism (CS-0050, CS-0060)
*   **Control Flow**: No `goto`. Loops/recursion MUST have hard limits and throw on breach (`CS-0010.37-38`, `CS-0060`).
*   **Safety Revolution**: Leverage C++26 verifiable safety and explicit intent. Prefer explicit composition over complex inheritance (`CS-0050`).
*   **Deterministic Logic**: Follow high-integrity standards (MISRA/JSF inspired) to ensure decidability and predictability (`CS-0060`).

### 3. Modifications & Feature Protection (CS-0030, CS-0040)
*   **Deletions**: No unjustified deletions. Any signature removal requires documented justification and caller/callee impact analysis (`CS-0030.3-5`).
*   **Annotations**: All methods must have Doxygen comments and `@feature` annotations detailing their contribution (`CS-0010.45`, `CS-0030.1`).
*   **Constants**: No "magic numbers". All system limitations and constants MUST be explicitly defined in dedicated headers (`CS-0040`).

### 4. Agent Operational Standards (CS-0070)
*   **Clearance**: Agents MUST NOT `commit` or `push` without explicit user clearance per task (`CS-0070.1`).
*   **Validation**: Every submission MUST be preceded by:
    1.  Successful local build (`CS-0070.3`).
    2.  Successful test pass of all relevant suites (`CS-0070.4`).
    3.  Deletion analysis report via `helpers/detect_deletions.py` (`CS-0070.5`).
*   **Review**: Agents MUST provide an impact summary and recommend manual review for all changes (`CS-0070.6-7`).

---

## 🚀 Active Roadmap & Task Context

### ⏸️ Stand-by Tasks (On Hold)
- **[TSK-20260410-002]**: **ENI Engineering**: ETG.2100 generation and Hot Connect topology resilience.
- **[TSK-20260410-005]**: **Master Redundancy**: Passive monitoring and redundancy ring failover logic.

### 📋 Backlog (Not Started)
- **[TSK-20260303-004]**: **Python Bindings**: C++ bindings for scripting integration.
- **[TSK-20260311-006]**: **CAN Bus / SocketCAN**: Hardware abstraction and DBC signal decoder interface.
- **[TSK-20260410-003]**: **ESI Cache Management**: Local index and resolver for vendor XML files.

### ✅ Recently Completed
- **[TSK-20260529-001]**: **Raw Ethernet Socket Service**: Dynamic packet capture and TreeTransformer integration.
- **[TSK-20260421-001]**: **Interactive Lua shell (`qlsh`)**: Command line REPL for Quasar script testing.
- **[TSK-20260410-004]**: **Interactive TUI (`ecat-top`)**: Keyboard-driven bus orchestration over SSH.
- **[TSK-20260410-001]**: **Reflexive EtherCAT Master**: Specialized diagnostic tools and reflexive slave management.
- **[TSK-20260328-001]**: **Reflexive Execution & Service Orchestration**: NamedMethod and NamedService implementation.
- **[TSK-20260311-011]**: **Lua Environment Validation**: Intensive thread stress testing.
- **[TSK-20260311-010]**: **Calibration Framework**: Polynomial/Point-Pair transforms.
- **[TSK-20260311-005]**: **OPC UA Integration**: Namespace 1 mirroring and recursive discovery.
- **[TSK-20260311-001]**: **Tree Transformation Engine**: XSLT-inspired rule matching.
- **[TSK-20260310-001]**: **Standalone Script Runner (`sre`)**: Plugin support launcher.

---

## 🔍 Verification Checklist
- [ ] Code strictly follows CS-0010 (Check: No `auto`? No `new`?).
- [ ] Doxygen comments on all public members.
- [ ] Timed mutexes used for all shared field modifications.
- [ ] Hard iteration limits on all loops/recursions.
- [ ] ASan and TSan pass without warnings.
