# Analysis of Missing Features in `sim`, `sched`, and `utils` Projects 

Based on the baseline SMP simulator report ([smp_simulator_report.md.resolved](file:///home/vortigern/git/quasar/tmp/smp_simulator_report.md.resolved)), the `sim` project, along with its core dependencies `sched` and `utils`, implements a foundational subset of the SMP2 standard. 

Unlike the `simtg-2A.7` framework which adapts an existing proprietary kernel to the SMP standard via wrappers (e.g., `LoggerAdapter`), the Quasar projects (`sim`, `sched`, `utils`) take a *native* approach, directly implementing the `Smp::Services::*` interfaces in C++. 

However, despite providing these core services natively, the ecosystem still lacks several critical features and capabilities necessary for a fully-fledged simulation framework. The missing capabilities have been categorized into **Abstractions** and **Implementations**.

---

## 1. Provided Capabilities (Context)

To understand what is missing, it is crucial to recognize what `sched` and `utils` actually provide:
- **`utils`**: Natively provides [ILogger](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/smp/src/Simulator.cpp#52-53), [ITimeKeeper](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/smp/src/Simulator.cpp#54-55) (managing simulation, epoch, mission, and zulu time), and `IEventManager` (managing global event subscriptions and emissions).
- **`sched`**: Provides a surprisingly complete, thread-safe (mutex-protected) `IScheduler`. It maintains timeline and immediate event queues for Simulation, Mission, and Epoch time events.

## 2. Missing Abstractions (`include/...`)

The following structural abstractions and interfaces are either missing or incompletely defined in the headers across the projects:

- **Model Execution Context ([SmpModelWrapper](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/smp/include/simtg/smp/SmpModelWrapper.hpp#32-354) equivalent)**:
  - **Missing Feature**: There is no abstraction to wrap an SMP [IModel](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/smp/src/Simulator.cpp#62-65) into a framework-specific execution or threading entity. In `simtg`, models are adapted to become OS-level `SchedulableObject`s. In Quasar, `sim::Model` directly implements `Smp::IModel`, leaving the execution context entirely up to the caller without a dedicated standard abstraction for parallelization.

- **Dataflow and Type Registration Abstractions (`ITypeRegistry`)**:
  - **Missing Feature**: While basic `IField` tracking exists in the headers, abstractions for complex data types, arrays, structures, and robust type registration (`Smp::Publication::ITypeRegistry`) are missing. Operations expecting types will find `Simulator::GetTypeRegistry()` merely returns `nullptr`.

---

## 3. Missing Implementations (`src/...`)

The actual executable kernel logic in the [.cpp](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Model.cpp) files is significantly simplified and lacks the following operational capabilities:

- **Dynamic Model Loading Engine ([LibraryLoader](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/kernel/include/simtg/kernel/LibraryLoader.hpp#63-67))**:
  - **Missing Capability**: The ability to dynamically load `.so` or `.dll` files containing compiled SMP models at runtime.
  - **Implementation Deficit**: The method `Simulator::LoadLibrary` in [Simulator.cpp](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Simulator.cpp) is an empty stub. The low-level `dlopen`/`dlsym` OS machinery needed to map into address space and resolve the required [Initialise](file:///home/vortigern/git/quasar/tmp/simtg-2A.7/smp/src/Simulator.cpp#418-488) and `Finalise` C-linkage entry points is entirely absent.

- **Field/Property Storage and Data Access Memory Backing**:
  - **Missing Capability**: Allowing the simulator or external clients to read/write state parameters.
  - **Implementation Deficit**: In `sim::Model` and `sim::Simulator`, methods like [GetSimpleValue](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Model.cpp#73-76), [SetSimpleValue](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Simulator.cpp#87-88), [GetField](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Model.cpp#62-65), and their array equivalents blindly return `nullptr` or throw `core::InvalidFieldName`. There is no runtime memory storage layout or lookup map bridging published field strings to physical memory addresses.

- **Robust State Machine Lifecycle Orchestration**:
  - **Missing Capability**: Properly passing state interfaces during model transitions.
  - **Implementation Deficit**: The [Simulator](file:///home/vortigern/git/quasar/cmake-projects/sim/include/sim/Simulator.hpp#29-30) attempts to orchestrate transitions via recursive helpers (e.g., [RecursivelyPublish](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Simulator.cpp#261-275), [RecursivelyConfigure](file:///home/vortigern/git/quasar/cmake-projects/sim/src/Simulator.cpp#276-293)), but the context injection is incomplete. For instance, the `component->Publish(...)` call inside `Simulator::RecursivelyPublish` is commented out or missing an actual `IPublication` receiver implementation. 

- **Advanced Execution Architecture**:
  - **Missing Capability**: While `sched::Scheduler` organizes the timeline, `Simulator::Run()` drives it via a basic, single-threaded infinite `while (_scheduler->ExecuteNextEvent() >= 0)` loop. It lacks the intricate synchronization mechanisms to pause/resume cleanly if user interfaces or external realtime triggers dictate the execution pace.

## 4. Conclusion

The Quasar `sim` infrastructure natively implements the core SMP services (`sched`, `utils`) without adapters, providing a clean baseline. However, to execute complex external SMP models similarly to `simtg-2A.7`, it requires immediate implementations for **Dynamic Library Loading (`dlopen`)**, **State Memory Backing (Dataflow)**, and completion of the **Initialization Lifecycle context mapping (`IPublication`)**.
