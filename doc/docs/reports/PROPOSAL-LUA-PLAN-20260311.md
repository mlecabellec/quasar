# Proposal: Lua Engine Validation & Enhancement (TSK-20260311-011)

This proposal outlines a phased implementation strategy for hardening the Quasar Lua execution environment and providing comprehensive bindings for the core system.

## 1. Impact Analysis

### 1.1. Core Type Coverage
**Status:** Incomplete. Currently, only basic numeric types and a few `NamedObject` variants are bound.
**Impact:** High. Many industrial protocols (EtherCAT, CAN) rely on `Buffer` and `BitBuffer` which are currently inaccessible from Lua.
**Solution:** Expand `RegistryBindings.cpp` to include the full spectrum of `NamedObject` subclasses.

### 1.2. ActiveEntity Reflexivity
**Status:** Unbound. Lua cannot currently control entity lifecycles or execute reflexive methods.
**Impact:** Critical. The "Logic Engine" and "Web UI" (TSK-20260311-009/008) directly depend on this capability.
**Solution:** Map `ActiveEntity` and its state machine to `sol::usertype` with protected execution hooks.

### 1.3. Asynchronous Reactivity (Observers)
**Status:** Missing bridge. C++ events cannot be handled by Lua functions.
**Impact:** High. Asynchronous system monitoring is currently impossible via scripts.
**Solution:** Implement `LuaObserver`, a C++ wrapper for `sol::function` that safely marshals events from C++ worker threads into the Lua state.

### 1.4. Thread Safety (TSan)
**Status:** The Lua engine is currently single-threaded in its standard usage.
**Impact:** C++ `ActiveEntity` notifications happen on arbitrary threads. Calling into Lua from these threads without synchronization will cause race conditions.
**Solution:** Introduce a global mutex in `LuaEngine` or implement a message-queue pattern to execute Lua callbacks on the main script thread.

### 1.5. Plugin Hot-Reloading
**Status:** `PluginLoader` supports loading but lacks stable unloading/reloading logic.
**Impact:** Medium. Useful for development but high-risk for production if stale C++ types remain in Lua.
**Solution:** Implement a `Unregister` protocol for plugins and ensure `ObjectTracker` orphans all plugin-created objects during reload.

## 2. Phased Implementation Plan

### Phase 1: The Proxy Foundation (Coverage)
*   **1.1 Proxy Design Pattern:** Implement a `LuaProxy<T>` layer that wraps C++ `NamedObject` hierarchies. This decorrelates the internal registry from the script representation, preventing scripts from accidentally corrupting the core topology while providing a clean, script-oriented API.
*   **1.2 Extended Type Bindings:** Add proxies for `NamedBoolean`, `NamedString`, `NamedBuffer`, `NamedBitBuffer`, `NamedDate`, `NamedTimestamp`, and `NamedDuration`.
*   **1.3 Collection Proxies:** Support `NamedArray`, `NamedMap`, and `NamedSet` with Lua-idiomatic table-like proxies.
*   **1.4 ActiveEntity Proxy:** Expose lifecycle hooks and the `execute()` reflexive method through the proxy.

### Phase 2: Reactivity & Thread-Per-Script
*   **2.1 Isolated Execution:** Transition to a **Thread-Per-Script** model. Each `LuaService` owns its `LuaEngine` and runs in a dedicated thread.
*   **2.2 Watermarked Event Queue:** Implement a `QueuedObserver` bridge. Events are pushed to a thread-safe queue with a configurable **High Watermark**.
    *   *Normal operation*: Events are queued and processed asynchronously by the script thread.
    *   *Watermark reached*: New events are dropped only when the queue exceeds the limit (emergency measure).
*   **2.3 Unrestricted Environment:** Remove sandboxing restrictions. Scripts have full access to standard Lua libraries including `io`, `os`, and `package`.

### Phase 3: Validation & Stress (Harden)
*   **3.1 Comprehensive Test Suite:** Implement `TSK-20260311-011.1-3` using the new Proxy architecture.
*   **3.2 Concurrency Stress:** Verify the isolation of the Thread-Per-Script model under high event load.
*   **3.3 Sanitizer Runs:** Validate ownership semantics of Proxies under ASan/TSan.

### Phase 4: Plugin Enhancement (Extend)
*   **4.1 Hot-Reload:** Implement `PluginLoader::unloadPlugin`.
*   **4.2 Type Stubs:** Generate `.pyi` / Lua definition files for better IDE support.

## 3. Confirmed Design Decisions (Technical Baseline)

*   **Concurrency Model**: **Thread-Per-Script**. High isolation, deterministic performance per module.
*   **Security Policy**: **No Sandboxing**. Scripts are trusted components with full system access.
*   **Interaction Pattern**: **Proxy Design Pattern**. Strict decorrelation between C++ core and Lua engine.
*   **Reactivity**: **Asynchronous Queuing with High Watermark**. Balancing deterministic delivery with memory safety.

---
> [!IMPORTANT]
> This plan ensures that Phase 1 and 2 unblock all high-level connectivity tasks (OPC UA, ZMQ, Web UI) by providing the necessary "glue" code first.
