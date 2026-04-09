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

## 🛠️ Mandatory Technical Standards (CS-0010)
*Violation of these rules constitutes a failure of structural integrity.*

### 1. Memory & Ownership
*   **Forbidden Keywords**: `new`, `delete`, `malloc`, `free`, `calloc`, `realloc`, `strdup`, `strndup` (`CS-0010.10-13`).
*   **Move Semantics**: Mandatory for passing by value (`CS-0010.2`) and returning objects (`CS-0010.4`).
*   **References**: Use `const` references instead of pointers for parameters whenever possible (`CS-0010.3`).
*   **Smart Pointers**: `shared_ptr` for shared lifecycles, `unique_ptr` for non-shared fields (`CS-0010.6-7`).
*   **Zero/One/Three**: Follow for constructor parameters (`CS-0010.9`).

### 2. Safety & Control Flow
*   **NO `auto`**: Explicit type declarations are strictly mandated (`CS-0010.34`).
*   **Forbidden Flow**: `goto` keywords (`CS-0010.17`) and unbounded arrays (`CS-0010.16`).
*   **Wait/Sleep**: Forbidden without timeouts. Use timed mutexes (`CS-0010.26-27`).
*   **Loop Limits**: Hard limits on all loops and recursion; throw exception on limit breach (`CS-0010.37-38`).
*   **Exceptions**: Mandatory handling for all potentially throwing functions (`CS-0010.20`).

### 3. Concurrency & Integrity
*   **Timed Mutexes**: Mandatory for all shared field modifications (`CS-0010.46`).
*   **RAII**: Mandatory for mutexes, files, sockets, and memory-mapped files (`CS-0010.21-24`).
*   **Refinement**: Literal -> Constant -> Enum -> Template -> Concept -> Trait progression (`CS-0010.39-43`).

### 4. Documentation & Clarity
*   **Comments**: Every code block requires an explanatory comment; max 5 lines without comments (`CS-0010.44`).
*   **Doxygen**: Mandatory for every class, field, and method (`CS-0010.45`).
*   **Functions**: Maximum 200 lines. Classes: Maximum 1600 lines.

---

## 🚀 Active Roadmap & Task Context

### Ongoing & In Progress
- **[TSK-20260311-001]**: Tree Transformation Engine (XSLT-inspired rule matching).
- **[TSK-20260311-004]**: ZeroMQ PUB/SUB Plugin for `NamedBuffer` distribution.
- **[TSK-20260311-010]**: Calibration Framework (Polynomial/Point-Pair transforms).
- **[TSK-20260311-011]**: Lua Environment Validation (Intensive stress testing).
- **[TSK-20260409-001]**: Global Logging Singleton & Macros (LOG_INFO, etc.).

### Current Directive
- **[TSK-20260328-001]**: **Reflexive Execution & Service Orchestration**.
    - Implementing `NamedMethod`, `NamedLuaMethod`, and `NamedService`.

### Recently Completed
- **[TSK-20260310-001]**: Standalone Script Runner (`sre`) with plugin support.
- **[TSK-20260308-001]**: Full Polymorphic Integer Dispatch.

---

## 🔍 Verification Checklist
- [ ] Code strictly follows CS-0010 (Check: No `auto`? No `new`?).
- [ ] Doxygen comments on all public members.
- [ ] Timed mutexes used for all shared field modifications.
- [ ] Hard iteration limits on all loops/recursions.
- [ ] ASan and TSan pass without warnings.
