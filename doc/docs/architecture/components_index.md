# Navigation by System Components

This index organizes Quasar framework files and specifications based on namespace and runtime modules.

---

## 🌲 Object Registry Namespace (`quasar::named`)

Encapsulates tree-based object discovery, dynamic interaction, and reflexivity:
- **Base Object Model**: `NamedObject`, coordinating hierarchical child containers and recursive thread mutexes.
- **Active State Manager**: `ActiveEntity`, providing HSM hooks and lifecycle callbacks.
- **Primitive Mappings**: `NamedPrimitive<T>`, providing zero-copy direct off-heap binary segment access.
- **Service Hooks**: `NamedService`, defining base interfaces for startup and teardown cycles.
- **Reference Diagram**: [Core Hierarchy Class Diagram](diagrams.md#core-hierarchy)
- **Base Specifications**: [CP-0010: Core Primitive Types](CP-0010.md)

---

## 📜 Scripting Namespace (`quasar::scripting`)

Provides sandboxed embedded logic, Lua bindings, and event-driven automation callbacks:
- **Script Executor**: `LuaEngine`, wrapping safe `sol::state` runtimes.
- **Weak Wrappers**: `LuaProxy<T>`, shielding direct C++ pointers from Lua memory sweeps.
- **Script Methods**: `NamedLuaMethod`, executing dynamic Lua script text.
- **Reference Diagram**: [Scripting Engine Class Diagram](diagrams.md#core-hierarchy)
- **Lua API Manual**: [Lua API Integration Specs](../manual/README.md)

---

## ⚡ Logic namespace (`quasar::logic`)

Manages deterministic scheduling loops, State Patterns, and hierarchical handshakes:
- **Scheduler Components**: `LogicComponent` (ActiveEntity extension), defining deterministic `step(dt)` cycles.
- **Scheduling Coordinator**: `LogicEngine`, executing logic loops inside bounded thread configurations.
- **State Handshaker**: `StateMachine` (HSM), wrapping State patterns for automations.
- **Reference Diagram**: [Logic Engine HSM Overview](diagrams.md#logic-engine-models)
- **Scheduling Rules**: [CS-0060: Bounded Logic Scheduler Standards](CS-0060.md)
