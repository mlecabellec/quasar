# Tutorial: Using NamedService from Lua

This tutorial explains how to create and manage autonomous background services using `NamedService` and the Quasar Script Runner (`sre`).

## Introduction

`NamedService` is a specialized `NamedObject` that encapsulates autonomous logic. It runs a dedicated background thread that executes lifecycle hooks defined as child methods. In the Lua environment, these hooks can be implemented as standard Lua functions.

## 1. The Script Runner (`sre`)

The Quasar script runner (`sre`) is a standalone executable used to run Lua scripts within the Quasar environment. It automatically loads core bindings (`named`, `coretypes`) and can dynamically load plugins.

### Basic Invocation
```bash
./build/bin/sre your_script.lua
```

### Invocation with Plugins
If your script requires external modules (like ZeroMQ), use the `--plugin` flag:
```bash
./build/bin/sre --plugin ./build/lib/quasar_zmq.so your_script.lua
```

## 2. Lua API for NamedService

### Creation
Create a service using the `quasar.named` module:
```lua
local service = quasar.named.createService("MyService", parentObject)
```

### Lifecycle Hooks
`NamedService` automatically looks for and executes specific child methods by name:
*   **`onStart`**: Executed once when the service starts.
*   **`run`**: Executed repeatedly in the service loop.
*   **`onStop`**: Executed once when the service stops.

Register these hooks using `createLuaMethod`:
```lua
quasar.named.createLuaMethod("run", function(owner, args)
    -- Your logic here
end, service)
```

### Control Methods
*   `service:start()`: Launches the background thread.
*   `service:stop()`: Signals the thread to stop and joins it.
*   `service:setCycleTime(ms)`: Sets the frequency of the `run` loop.
*   `service:isRunning()`: Returns the execution state.

## 3. Full Example

This script demonstrates a telemetry service that increments a counter and publishes data via ZMQ.

```lua
-- telemetry_service.lua
io.stdout:setvbuf("no") -- Disable buffering for immediate logs

local function log(msg)
    io.write(msg .. "\n")
    io.stdout:flush()
end

log("--- Starting NamedService Demo ---")

-- 1. Setup Hierarchy
local root = quasar.named.createObject("root", nil)
local service = quasar.named.createService("TelemetrySvc", root)
local counter = quasar.named.createLong("iterations", 0, service)

-- 2. Setup ZMQ (Requires --plugin quasar_zmq.so)
local ctx = quasar.zmq.Context()
local pub = ctx:socket(quasar.zmq.PUB)
pub:bind("tcp://*:5555")

-- 3. Define Hooks
quasar.named.createLuaMethod("onStart", function(owner, args)
    log("[onStart] Service " .. owner:getName() .. " starting...")
end, service)

quasar.named.createLuaMethod("run", function(owner, args)
    local c = owner:getChild("iterations"):asLong()
    local newVal = c:value() + 1
    c:setValue(newVal)
    
    -- Publish via ZMQ
    pub:send("telemetry: " .. tostring(newVal))
    log("[run] Iteration: " .. newVal)
end, service)

quasar.named.createLuaMethod("onStop", function(owner, args)
    log("[onStop] Service stopped. Final count: " .. owner:getChild("iterations"):asLong():value())
end, service)

-- 4. Configure and Start
service:setCycleTime(1000) -- 1 second loop
service:start()

-- 5. Keep main script alive
log("Monitoring for 10 seconds...")
local startTime = os.time()
while os.difftime(os.time(), startTime) < 10 do
    os.execute("sleep 0.5") -- Yield to background thread
end

-- 6. Cleanup
log("Shutting down...")
service:stop()
log("--- Demo Finished ---")
```

## 4. Key Notes
*   **Thread Safety**: The `run` hook executes in a separate thread. Ensure any shared data access is handled via `NamedObject` methods, which are internally protected by mutexes.
*   **Blocking**: Avoid long blocking calls inside the `run` hook, as this will delay the next cycle and may interfere with service termination.
*   **Output**: When running via `sre`, use `io.write()` and `io.stdout:flush()` for reliable log output from background threads.
