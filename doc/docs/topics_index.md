# Navigation by Developer Topics

This index organizes the Quasar framework documentation by developer topics and system capabilities.

---

## 🌲 Topic A: Named Tree & Data Modeling
Focuses on Quasar's hierarchical object tree, non-destructive memory mapping, and primitive types.
- [Architecture Elements Overview](architecture/README.md): System components, coding constraints, and designs registry.
- [CP-0010: Core Primitive Types](architecture/CP-0010.md): Specifications for named primitives and memory boundaries.
- [Implementation Specifications](impl/README.md): Class implementations, code coverage indices, and justifications.

---

## 📜 Topic B: Embedded Lua Scripting Engine
Focuses on the sandboxed Lua runtime executor, dynamic reflexivity, and event-driven automation callbacks.
- [Lua Integration Manual](manual/README.md): Step-by-step Lua integration hello-world, serializations, and test cases.
- [Lua Service Framework](manual/named-service-lua.md): Running Lua-based background services and callbacks.

---

## ⚖️ Topic C: C++ Standards & Code Regulations
Focuses on C++23 standards, dynamic allocations ban, thread safety, and code modifications validation.
- [CS-0010: C++ Coding Standards](architecture/CS-0010.md): Basic compliance rules (explicit types, banned raw pointers, banned native arrays).
- [CS-0020: Advanced C++ Standards](architecture/CS-0020.md): Thread safety models,Timed recursive mutexes, exception safety.
- [CS-0030: Code Modification Standards](architecture/CS-0030.md): Code deletion analysis, impact studies, and peer-review gateways.
- [CS-0050: C++26 Safety Standards](architecture/CS-0050.md): Memory safety, static analysis, and ahead-of-time compilations.

---

## ⚡ Topic D: Logic Scheduler & Network Transports
Focuses on active entities cycles, hierarchical state machines, and high-integrity ZeroMQ sockets.
- [CS-0060: Bounded Logic Scheduler Standards](architecture/CS-0060.md): GC-flat loops, state patterns, and deterministic schedulers.
- [CS-0070: High-Integrity ZeroMQ Standards](architecture/CS-0070.md): Safe sockets wrappers, transport boundaries, and message streams.
- [Architecture diagrams](architecture/diagrams.md): Class model diagrams for named objects, scripting proxies, and logic HSMs.

---

## 🐧 Topic E: Linux Kernel Modules & Distro Ports
Focuses on low-level industrial hardware drivers, kernel models, and OS requirements.
- [Debian Linux Porting Specs](DEBIAN-REQUIREMENTS.md): Debian Bookworm compilation targets, toolchains, and package naming rules.
- [Fedora Linux Porting Specs](FEDORA-REQUIREMENTS.md): Fedora packages and kernel requirements.
- [Arch Linux Porting Specs](ARCHLINUX-REQUIREMENTS.md): PKGBUILD guidelines and dependencies.
