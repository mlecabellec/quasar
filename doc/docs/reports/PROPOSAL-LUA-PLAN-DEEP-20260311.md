# Deep Analysis & Phased Implementation Plan: Lua Engine Validation (TSK-20260311-011)

## 1. Executive Summary
This document provides a deep technical analysis and a comprehensive 4-phase plan to elevate Quasar's Lua integration to industrial standards. Core improvements include a **Proxy Design Pattern** for memory safety, a **Thread-Per-Script** model for deterministic execution, and **Watermarked Asynchronous Queues** for reactive event handling.

---

## 2. Deep Impact Analysis

### 2.1 Memory Management & Object Lifetime
**Observation:** The current binding directly exposes `std::shared_ptr<NamedObject>` to Lua. If a Lua script keeps a reference to a temporary object, it prevents C++ from reclaiming memory.
**Impact:** Stale references causing silent memory growth and potential logic errors when objects are "logically" deleted but "physically" kept alive by Lua.
**Solution (Proxy Pattern):**
*   Lua will only interact with `LuaProxy<T>` objects.
*   Each Proxy holds a `std::weak_ptr<T>`.
*   Before any operation, the Proxy attempts to `lock()` the pointer. If it fails, a Lua error is raised ("Access to invalidated object").
*   **Result:** C++ maintains absolute control over lifecycle; Lua is informatively notified of object destruction.

### 2.2 Concurrency & Thread Isolation
**Observation:** Quasar is highly concurrent (asio workers, fieldbus threads). Global Lua state sharing would require heavy mutex locking, causing jitter.
**Impact:** High GC pauses or lock contention in one script could block real-time networking or control logic in others.
**Solution (Thread-Per-Script):**
*   Each `LuaService` runs its own OS thread.
*   Each thread owns a private `sol::state`.
*   Cross-service communication happens exclusively through `ActiveEntity` events or the global `NamedObject` tree (protected by existing C++ recursive mutexes).
*   **Result:** Deterministic performance; a crash or hang in one script is isolated to its service.

### 2.3 Reactive Event Marshalling
**Observation:** `ActiveEntity` observers notify via virtual calls on the producer's thread. Direct Lua calls from these threads violate the Thread-Per-Script isolation.
**Impact:** Race conditions if Lua is accessed from multiple threads simultaneously.
**Solution (Queued Observer):**
*   A `QueuedObserver` bridge pushes events into a thread-safe `SPSCQueue` (Single Producer Single Consumer).
*   The `LuaService` thread polls this queue at the start of its update loop.
*   **High Watermark Logic:** If the queue exceeds `MAX_EVENTS` (e.g., 10,000), new events are dropped, and a "Queue Overflow" warning is logged.
*   **Result:** Thread-safe, non-blocking asynchronous reactivity.

### 2.4 Performance & Overhead
**Impact Assessment:**
*   **CPU:** Proxy indirection adds ~10-15ns per call. In the context of Lua execution (~500ns for a simple table access), this is negligible (<3%).
*   **Memory:** Each Proxy adds ~32 bytes (weak_ptr + metadata). 100,000 objects = ~3MB overhead.
*   **Binary Size:** Expanded bindings for 15+ types will add ~1.2MB to the `libquasar_scripting.so` binary.

---

## 3. Phased Implementation Plan

### Phase 1: Core Proxy Architecture (Hardening)
**Objective:** Establish the secure interaction baseline.

1.  **[NEW] `quasar::scripting::LuaProxy<T>`**:
    *   Implement base template for weak-reference management.
    *   Implement `isAlive()` and `invalidate()` hooks.
2.  **[MODIFY] `RegistryBindings.cpp`**:
    *   Refactor to return `LuaProxy<NamedObject>` for all tree traversals.
3.  **[EXTEND] Full Type Coverage**:
    *   Implement specialized proxies for: `NamedBuffer`, `NamedBitBuffer`, `NamedInteger`, `NamedFloatingPoint`, `NamedQuantity`, `NamedString`, `NamedBoolean`, `NamedDate`, `NamedTimestamp`, and `NamedDuration`.
4.  **[MODIFY] Collection Proxies**:
    *   Enable table-style access for `NamedArray` and `NamedMap` (e.g., `myArray[1]` -> proxy for element).

### Phase 2: Reactivity & Isolation (Threading)
**Objective:** Implement the industrial execution model.

1.  **[MODIFY] `LuaService` Threading**:
    *   Add `std::thread m_workerThread`.
    *   Implement loop: `while(m_running) { processEvents(); onUpdate(dt); gcStep(); }`.
2.  **[NEW] `QueuedObserver`**:
    *   Thread-safe queue with configurable High Watermark (default 10k).
    *   Atomic counter for dropped events.
3.  **[MODIFY] Unrestricted Sandbox**:
    *   Open all `sol::lib` modules.
    *   Ensure `io.*` and `os.*` are available for diagnostic scripts.

### Phase 3: Reflexivity & Control (Expansion)
**Objective:** Enable high-level logic and UI integration.

1.  **[MODIFY] `ActiveEntity` Proxy**:
    *   Bind `initialize()`, `start()`, `stop()`, `reset()`.
    *   Bind `execute(methodName, args)` for reflexive command execution.
2.  **[EXTEND] Field Reflexivity**:
    *   Expose `getField(name)` and `listFields()` via Proxy properties.
3.  **[NEW] Command Proxy**:
    *   Implement `LuaProxy<ICommand>` to allow executing C++ commands from Lua.

### Phase 4: Verification & Tooling (Validation)
**Objective:** Ensure quality and developer productivity.

1.  **[NEW] Stress Test Suite**:
    *   Create 1M `NamedObject` proxies and verify no memory leaks.
    *   Verify `TSan` cleanliness during high-frequency event bursts.
2.  **[NEW] Hot-Reload Verification**:
    *   Verify that if a C++ plugin is unloaded, all associated Lua proxies correctly report `isAlive() == false`.
3.  **[NEW] Metadata Generation**:
    *   Generate Lua Type Definition files (LDoc or EmmyLua) to enable Autocomplete/IntelliSense in VS Code.

---

## 4. Resource & Risk Mitigation

| Risk | Mitigation Strategy |
| :--- | :--- |
| **Deadlocks** | Proxies never hold locks across Lua calls. They lock, copy/operate, and release immediately. |
| **GC Jitter** | Service updates use `LuaEngine::gcStep()` to perform incremental collection, avoiding major stalls. |
| **OOM due to Queues** | The High Watermark is strictly enforced. No unbounded growth allowed. |

---
> [!IMPORTANT]
> This detailed plan ensures Quasar can handle high-frequency industrial data (via specialized bit/buffer proxies) while maintaining the safety of a managed memory environment.
