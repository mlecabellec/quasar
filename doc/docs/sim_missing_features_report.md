# Analysis of Missing Features in `sim`, `sched`, and `utils` Projects 

Based on the baseline SMP simulator report (`tmp/smp_simulator_report.md.resolved`), the `sim` project, along with its core dependencies `sched` and `utils`, implements a foundational subset of the SMP2 standard. 

Unlike the `simtg-2A.7` framework which adapts an existing proprietary kernel to the SMP standard via wrappers (e.g., `LoggerAdapter`), the Quasar projects (`sim`, `sched`, `utils`) take a *native* approach, directly implementing the `Smp::Services::*` interfaces in C++. 

However, despite providing these core services natively, the ecosystem still lacks several critical features and capabilities necessary for a fully-fledged simulation framework. The missing capabilities have been categorized into **Abstractions** and **Implementations**.

---

## 1. Provided Capabilities (Context)

To understand what is missing, it is crucial to recognize what `sched` and `utils` actually provide:
- **`utils`**: Natively provides `ILogger`, `ITimeKeeper` (managing simulation, epoch, mission, and zulu time), and `IEventManager` (managing global event subscriptions and emissions).
- **`sched`**: Provides a surprisingly complete, thread-safe (mutex-protected) `IScheduler`. It maintains timeline and immediate event queues for Simulation, Mission, and Epoch time events.

## 2. Missing Abstractions (`include/...`)

The following structural abstractions and interfaces are either missing or incompletely defined in the headers across the projects:

- **Model Execution Context (`SmpModelWrapper` equivalent)**:
  - **Missing Feature**: There is no abstraction to wrap an SMP `IModel` into a framework-specific execution or threading entity. In `simtg`, models are adapted to become OS-level `SchedulableObject`s. In Quasar, `sim::Model` directly implements `Smp::IModel`, leaving the execution context entirely up to the caller without a dedicated standard abstraction for parallelization.

- **Dataflow and Type Registration Abstractions (`ITypeRegistry`)**:
  - **Missing Feature**: While basic `IField` tracking exists in the headers, abstractions for complex data types, arrays, structures, and robust type registration (`Smp::Publication::ITypeRegistry`) are missing. Operations expecting types will find `Simulator::GetTypeRegistry()` merely returns `nullptr`.

---

## 3. Missing Implementations (`src/...`)

The actual executable kernel logic in the `.cpp` files is significantly simplified and lacks the following operational capabilities:

- **Dynamic Model Loading Engine (`LibraryLoader`)**:
  - **Missing Capability**: The ability to dynamically load `.so` or `.dll` files containing compiled SMP models at runtime.
  - **Implementation Deficit**: The method `Simulator::LoadLibrary` in `cmake-projects/sim/src/Simulator.cpp` is an empty stub. The low-level `dlopen`/`dlsym` OS machinery needed to map into address space and resolve the required `Initialise` and `Finalise` C-linkage entry points is entirely absent.

- **Field/Property Storage and Data Access Memory Backing**:
  - **Missing Capability**: Allowing the simulator or external clients to read/write state parameters.
  - **Implementation Deficit**: In `sim::Model` and `sim::Simulator`, methods like `GetSimpleValue`, `SetSimpleValue`, `GetField` (and `sim/src/Model.cpp` / `sim/src/Simulator.cpp`), and their array equivalents blindly return `nullptr` or throw `core::InvalidFieldName`. There is no runtime memory storage layout or lookup map bridging published field strings to physical memory addresses.

- **Robust State Machine Lifecycle Orchestration**:
  - **Missing Capability**: Properly passing state interfaces during model transitions.
  - **Implementation Deficit**: The `Simulator` (`cmake-projects/sim/include/sim/Simulator.hpp`) attempts to orchestrate transitions via recursive helpers (e.g., `RecursivelyPublish`, `RecursivelyConfigure` in `cmake-projects/sim/src/Simulator.cpp`), but the context injection is incomplete. For instance, the `component->Publish(...)` call inside `Simulator::RecursivelyPublish` is commented out or missing an actual `IPublication` receiver implementation. 

- **Advanced Execution Architecture**:
  - **Missing Capability**: While `sched::Scheduler` organizes the timeline, `Simulator::Run()` drives it via a basic, single-threaded infinite `while (_scheduler->ExecuteNextEvent() >= 0)` loop. It lacks the intricate synchronization mechanisms to pause/resume cleanly if user interfaces or external realtime triggers dictate the execution pace.

## 4. Conclusion

The Quasar `sim` infrastructure natively implements the core SMP services (`sched`, `utils`) without adapters, providing a clean baseline. However, to execute complex external SMP models similarly to `simtg-2A.7`, it requires immediate implementations for **Dynamic Library Loading (`dlopen`)**, **State Memory Backing (Dataflow)**, and completion of the **Initialization Lifecycle context mapping (`IPublication`)**.
