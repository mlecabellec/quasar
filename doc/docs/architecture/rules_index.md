# Navigation by Project Rules & Guidelines

This index organizes the Quasar framework quality guidelines, coding safety rules, and project regulations.

---

## 📋 Quality Standards & Code Review

Regulations governing code ownership, commits, and review processes:
- **Standard**: [Constraint CS-0010 Quality Standards](CS-0010.md)
  - Explicit requirements/task mapping rules.
  - Verification gateways in git commits and merges.
- **Review & Modifications**: [Constraint CS-0030 Code Modification Standards](CS-0030.md)
  - Impact study rules for deletions and major edits.
  - Human validation checklists.
- **Commenting & Documentation**: [Constraint CS-0090 Formal C++ Commenting & Doxygen Standards](CS-0090.md)
  - Universal Doxygen coverage, traceability (`@task`/`@requirement`/`@constraint`/`@compliance`), and 4-part `@param` specs.
  - Mandatory in-body justification and control-structure operational comments.
- **Testing & Verification**: [Constraint CS-0100 Comprehensive Testing Standards](CS-0100.md)
  - Universal method coverage, traceability, branch/domain/template analysis, determinism, and resource limits (`2 GB` / `3 min`).
  - Concurrency/stress testing, cross-module integration graphs, progressive tutorial tests, async verbose logging, and PlantUML test docs.

---

## 💻 C++ Programming Standards

Regulations governing C++23 syntax constraints, thread mutexes, and compile-time checks:
- **Coding Standard**: [Constraint CS-0020 C++ Coding Standards](CS-0020.md)
  - Raw pointers ban, dynamic allocation check, and native arrays exclusion.
- **Safety Standard**: [Constraint CS-0050 C++26 Safety Standards](CS-0050.md)
  - Memory bounds assertions, static analyzer integration, and GraalVM/AoT compile gates.

---

## ⚙️ Logic Scheduling & Transport Standards

Regulations governing execution loops and sockets operations:
- **Logic Standards**: [Constraint CS-0060 Bounded Logic Scheduler Standards](CS-0060.md)
  - Bounded ring buffers allocations, state patterns implementation, and GC-flat execution.
- **ZeroMQ Standards**: [Constraint CS-0070 High-Integrity ZeroMQ Standards](CS-0070.md)
  - Safe RAII wrapper scopes, thread boundaries, and non-blocking transports.
