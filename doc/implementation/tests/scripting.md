# Scripting Module Tests

This document describes the testing suite for the `scripting` module, located in `cmake-projects/scripting/test/`.

## 1. Core Engine Tests (`TestLuaEngine.cpp`)
- **State Integrity**: Verifies that each `LuaEngine` instance is isolated and correctly initializes standard libraries.
- **Execution Safety**: Validates that `executeString` handles syntax errors and runtime exceptions without crashing the host process.
- **GC Determinism**: Measures the overhead of `gcStep` and verifies that memory is reclaimed correctly.

## 2. Service Lifecycle (`TestLuaService.cpp`)
- **Worker Isolation**: Verifies that each `LuaService` runs in its own thread and does not block others.
- **Task Posting**: Validates the thread-safe `postTask` mechanism for C++ to Lua communication.
- **State Persistence**: Ensures that Lua variables are preserved across multiple `onUpdate` cycles.

## 3. Bindings and Interop (`TestTypeBindings.cpp`)
- **NamedObject Bridge**: Verifies that all `coretypes` and `named` objects can be passed to and from Lua with full type fidelity.
- **Method Overrides**: Tests `ScriptableNamedObject` by overriding C++ virtual methods in Lua and verifying they are called correctly from C++.
- **Asynchronous Results**: Validates `LuaFuture` and the ability to return values from background script tasks.

## 4. Plugin System (`TestPluginLoading.cpp`)
- **Dynamic Registration**: Verifies that `PluginLoader` correctly loads shared libraries and registers new component types.
- **Hot-Reloading**: (Manual/Planned) Verification of system stability when plugins are swapped at runtime.
