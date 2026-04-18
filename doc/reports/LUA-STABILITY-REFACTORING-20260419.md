# Lua Engine Stabilization & Lifecycle Hardening Report

**Date:** 2026-04-19  
**Status:** 🔄 **In Progress (Refactoring Phase)**  
**Tasks:** Phase 1 (Ownership) & Phase 2 (Contextual Tracking)

## 1. Executive Summary
This report details the ongoing effort to resolve critical memory safety and concurrency issues in the Quasar Lua execution environment. The primary focus is shifting from raw pointer management to a shared ownership model with partitioned object tracking, ensuring that service destruction does not leave dangling references or cause collateral invalidation of unrelated scripts.

---

## 2. Problem Analysis Matrix

| Observed Problem | Root Cause | Proposed Solution | CS-0010 Compliance |
| :--- | :--- | :--- | :--- |
| **Memory Leak in `ObjectTracker`** | `m_anyStrongObjects` never purged; holds objects indefinitely. | Partition tracking by `EngineID`; implement `untrackAll(id)` on shutdown. | **CS-0010.6** (Lifecycle management). |
| **Use-after-free in `NamedLuaMethod`** | Held raw `LuaEngine*` became dangling on service stop. | Use `std::weak_ptr<LuaEngine>` with `lock()` check before execution. | **CS-0010.3** (Avoid raw pointers). |
| **Global Collateral Invalidation** | `LuaEngine` dtor killed ALL methods globally. | Refactored `invalidateMethods(id)` to only affect methods of a specific engine. | System structural integrity. |
| **Valgrind "Invalid read" at Shutdown** | Lua GC calling `__gc` after library deallocation. | Explicit `shutdown()` method to clear registry and run GC before dtor. | **CS-0010.21** (RAII/Explicit cleanup). |
| **Logic Engine Worker Instability** | Collision/Race on static usertype metadata. | Register usertypes once per worker; use `LogicProxy` with contextual ID. | **CS-0010.5** (Thread safety). |

---

## 3. Implementation Plan & Progress

### Phase 1: Ownership & Lifecycle (80% Complete)
- [x] **Step 1.1: `LuaEngine` Shared Ownership**: Migrated to `std::enable_shared_from_this` and factory patterns.
- [x] **Step 1.2: Method Safety**: Updated `NamedLuaMethod` to hold `weak_ptr<LuaEngine>`.
- [x] **Step 1.3: Explicit Shutdown**: Added `LuaEngine::shutdown()` called by `LuaService`.
- [ ] **Pending**: Final signature alignment for `execute` override in `NamedLuaMethod`.

### Phase 2: `ObjectTracker` Contextualization (70% Complete)
- [x] **Step 2.1: Contextual Tracking**: Added `EngineID` partitioning to the singleton tracker.
- [x] **Step 2.2: Scoped Cleanup**: Implemented `untrackAll(id)` for targeted resource release.
- [x] **Step 2.3: Type Binding Updates**: Updated all core type creation to pass the current engine ID.
- [ ] **Pending**: Update remaining networking and ZMQ plugins to use the new `trackStrong(id, ptr)` signature.

### Phase 3: Stabilization & Container Hardening (20% Complete)
- [x] **Step 3.1: Logic Proxy Registration**: Moved registration to worker init.
- [ ] **Step 3.2: sol2 Hardening**: Explicitly register common STL containers (`std::vector<uint8_t>`, etc.) to stabilize metadata access.

---

## 4. Current Blockers & Critical Issues

### 4.1 `NamedLuaMethod::execute` Signature Mismatch
The implementation in `NamedLuaMethod.cpp` currently conflicts with the declaration in `NamedLuaMethod.hpp`. This was caused during the transition to the new execution model. 
*   **Resolution Required**: Standardize on `std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) override`.

### 4.2 `WebServerWrapper` Type Errors
Refactoring the Networking plugin to support both secure (`SSLContext`) and non-secure servers introduced template instantiation errors in `lua.create_table_with`.
*   **Root Cause**: Incorrect namespace usage for `asio::ssl::context` vs `CppServer::Asio::SSLContext`.
*   **Resolution Required**: Surgical correction of `WebServerWrapper.hpp` and `.cpp` to use consistent types.

### 4.3 Test Ambiguity
`future.get()` calls in `TestLuaService.cpp` and `TestScriptExecution.cpp` are ambiguous for the compiler when dealing with `sol::protected_function_result`.
*   **Resolution Required**: Use explicit `result.get<T>()` or `static_cast<T>(result)`.

---

## 5. Handover Roadmap (Next Immediate Steps)
1.  **Repair Build**: Fix the signature mismatch in `NamedLuaMethod`.
2.  **Plugin Audit**: Verify all `ObjectTracker::trackStrong` calls in `cmake-projects/scripting/plugins/**` pass the mandatory `engineId`.
3.  **Valgrind Verification**: Execute `valgrind --leak-check=full ./bin/scripting_test` once compilation is restored to confirm 0 leaks in `ObjectTracker`.

---
*Report maintained by Quasar Engineering Agent.*
