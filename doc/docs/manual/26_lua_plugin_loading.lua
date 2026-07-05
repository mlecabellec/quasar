-- 26_lua_plugin_loading.lua
-- The Exoskeleton: Loading Plugins dynamically.
-- "Why carry all your tools when you can just pick them up when you need them?"

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Dynamic Plugin Loading ---")

-- 1. In Quasar, plugins are usually loaded via the `--plugin` flag in `sre`.
-- But inside Lua, we can check for their existence to see what we're working with.
local function checkPlugin(name, namespace)
    if quasar[namespace] then
        print("[Plugin] " .. name .. " is loaded and active. Ready to rock.")
        return true
    else
        print("[Plugin] " .. name .. " is missing. (Did you use --plugin build/lib/" .. name .. ".so?)")
        return false
    end
end

-- 2. Let's check our arsenal
print("\nChecking available extensions in the current environment:")
local hasZMQ = checkPlugin("quasar_zmq", "zmq")
local hasNet = checkPlugin("quasar_net_plugin", "net")
local hasOpc = checkPlugin("quasar_opcua_plugin", "opcua")
local hasDL  = checkPlugin("quasar_datalogger_plugin", "datalogger")

print("\nPlugin Summary:")
print("----------------")
print("ZMQ:        " .. tostring(hasZMQ))
print("Net:        " .. tostring(hasNet))
print("OPC UA:     " .. tostring(hasOpc))
print("DataLogger: " .. tostring(hasDL))
print("----------------")

print("--- Plugin Loading Finished ---")
-- "Remember: A plugin a day keeps the monolithic-architecture blues away."
