# 🌌 Quasar Scripting Module

The Quasar Scripting module provides a high-performance, thread-safe Lua environment integrated with the `NamedObject` hierarchy. It allows for reflexive logic, autonomous services, and dynamic extension of C++ components via Lua.

## 🚀 Key Features

- **Hierarchy Integration**: Direct access to the `NamedObject` tree from Lua.
- **Type Safety**: Automatic mapping between C++ `NamedInteger`, `NamedString`, etc., and Lua types via proxies.
- **Marshalling**: Transparent execution of Lua methods across thread boundaries when used with `LuaService`.
- **Dynamic Plugins**: Support for loading external C++ shared libraries that register new Lua bindings.
- **Object Tracking**: Built-in lifecycle management ensuring Lua proxies don't access destroyed C++ memory.

## 🧵 Threading & Locking Model

Quasar uses a **Single-Lock Multi-Threaded** model for Lua:
1. Each `LuaEngine` (and by extension each `LuaService`) is guarded by a **recursive mutex**.
2. Only one thread can execute Lua code in a given state at any time.
3. C++ threads (like `NamedService` background loops) must acquire this lock before calling a `NamedLuaMethod`.

### ⚠️ Critical Limitation: Blocking Calls
Since the Lua state is protected by a mutex, calling blocking functions like `os.execute("sleep ...")` or long-running synchronous I/O from within Lua will **starve the engine**.
- While the script is sleeping via `os.execute`, it still holds the engine lock.
- No other thread (e.g., a background `NamedService` running a `run` hook) can enter the Lua state.
- This leads to timeouts and perceived "stuck" scripts.

### ✅ Solution: Cooperative Multitasking (`quasar.sleep`)
To avoid lock starvation, always use `quasar.sleep(ms)` instead of `os.execute`.
- `quasar.sleep` explicitly **releases the engine lock** before sleeping.
- This allows background services to execute their hooks while the main script is waiting.
- The lock is automatically re-acquired before `quasar.sleep` returns to your script.

## 🛠️ Limitations & Known Issues

- **Global Lock**: High contention can occur if many background services try to execute Lua hooks simultaneously on the same state.
- **Marshalling Timeouts**: If a `LuaService` thread is busy, calls to its methods from other threads may timeout (default 10s).
- **Standalone Runner (`sre`)**: The standalone runner uses a single state. Background services created in `sre` share the lock with the main script.
- **External Libraries**: Standard Lua libraries like `io` and `os` are available but unaware of Quasar's locking model. Use them with caution.

## 📖 Best Practices

1. **Yield Often**: Use `quasar.sleep(0)` if you have a tight loop to allow other tasks to run.
2. **Handle Nil**: Network calls (ZMQ, OPC UA) are often non-blocking; always check if the returned object is `nil`.
3. **Proxy Integrity**: Use `obj:isAlive()` to check if a proxy still points to a valid C++ object if you store it long-term.
