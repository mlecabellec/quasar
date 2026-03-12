<div align="center">
  <img src="https://img.shields.io/badge/Quasar-Automation-blueviolet?style=for-the-badge&logo=rocket" alt="Quasar Logo Badge" />
  
  <h1>🌌 Quasar</h1>
  <p><b>Global Industrial Automation Solution</b></p>
  
  [![CI Build Status](https://github.com/mlecabellec/quasar/actions/workflows/ci.yml/badge.svg)](https://github.com/mlecabellec/quasar/actions/workflows/ci.yml)
  ![C++](https://img.shields.io/badge/C++23-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
  ![Lua](https://img.shields.io/badge/Lua-2C2D72?style=flat-square&logo=lua&logoColor=white)
  ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=flat-square&logo=cmake&logoColor=white)
  ![Linux Kernel](https://img.shields.io/badge/Linux_Kernel_Modules-FCC624?style=flat-square&logo=linux&logoColor=black)
</div>

---

## 🚀 About Quasar

**Quasar** is a cutting-edge global industrial automation solution designed to bridge the gap between high-level application development and low-level industrial hardware. By providing robust middleware, strict standard enforcement, and an extensible architecture, Quasar enables the rapid development of deterministic and secure automations.

### Core Objectives
- 🔌 **Middleware & Drivers**: Provide required middleware (including Linux kernel modules, userland libraries, and userland applications) to interact with common industrial hardware and standard protocols (e.g., EtherCAT).
- 🛠️ **Developer API**: Offer a highly-structured API-oriented interface for seamless application development.
- 🧪 **Hardare-In-The-Loop Simulation (HILS)**: Provide comprehensive environments and integration points for simulating common industrial hardware and protocols. Applications and automations run seamlessly whether in a simulated landscape or a real-world deployment.
- ⚡ **Rapid Development**: Feature intuitive environments heavily augmented with embedded scripting tools to accelerate automation logic authoring.

---

## ✨ Features Highlight

- **Tree Transformation & Data Mapping**: A sophisticated, non-destructive memory-slice mapping and XSLT-inspired tree transformation engine (`quasar::named`) to parse and project raw binary bytes natively into structured APIs.
- **Embedded Lua Scripting**: A fully sandboxed, asynchronous Lua integration allowing end-to-end reflexivity and extension of core Quasar concepts dynamically. 
- **Zero-Copy Memory Principles**: Engineered with `SHARE` paradigms where objects map directly over incoming buffers, obliterating serialization overheads.
- **Strict Compliance (`CS-0010`)**: 
  - Mandated **C++23** syntax and logic (`auto`, `new`/`delete`, raw pointers natively banned).
  - 100% thread safety requirement for shared data structures using timed recursive mutexes.
  - Zero tolerance policy for undefined behaviors, memory leaks, or unhandled states enforced strictly by Valgrind and CppCheck CI gates.

---

## 📂 Project Anatomy

Quasar comprises multiple projects and modules spanning varying distinct technologies. 

```text
quasar/
├── cmake-projects/    # Core C++ libraries and executables
│   ├── coretypes/     # Base primitive and data definitions
│   ├── named/         # Hierarchical object tree, transformation, and registry system
│   └── scripting/     # Embedded Lua service infrastructure
├── linux-modules/     # Linux kernel modules (e.g., EtherCAT master logic)
├── maven-projects/    # Java/Maven ecosystems
├── ext-projects/      # Third-party or decoupled external tools under varied build systems
└── doc/               # Comprehensive project documentation
    ├── architecture/  # Core constraints, design rules, and dependency matrices
    ├── features/      # Specifications for current and upcoming mechanics
    └── project/       # Task tracking and assignment definition directives
```

---

## 📚 Documentation Reference

To dive deeper into the project specifications, consult the `/doc` tree.
- [Architecture Details](doc/architecture)
- [Feature Breakdown](doc/features)
- [Task Log](doc/project/tasks)

> **Note to Contributors & Agents**: Always consult `doc/TODO.md` and `doc/project/` for short-term task horizons, and ensure absolute compliance with all definitions housed in `doc/architecture/CS-0010.md` before submitting code!
