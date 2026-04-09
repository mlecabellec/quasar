-- 23_data_logger_simple.lua
-- The Historian: Data Logging.
-- "Those who cannot remember the past are condemned to repeat it. 
-- In our case, we just log it."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: DataLogger Basics ---")
if not quasar.datalogger then
    print("ERROR: DataLogger plugin not loaded. Run with '--plugin build/lib/quasar_datalogger_plugin.so'")
    os.exit(1)
end

local root = n.createObject("LogSystem")
local pressure = n.createDouble("Pressure", 1.0, root)

-- 1. Create the DataLogger Service
print("Initializing DataLogger Service (Capacity: 100 entries)...")
local logger = quasar.datalogger.createService("PressureLogger", 100, root)

-- 2. Define the recording logic using a simple hook
-- Real DataLoggers in Quasar track objects automatically, but here we can simulate 
-- pulling data into the logger or using the logger's background loop.
n.createLuaMethod("run", function(owner, args)
    local p = owner:getParent():getChild("Pressure"):asDouble():value()
    -- Here we simulate the logger recording the value. 
    -- In a full C++ setup, you'd add recorders to the service.
    print("[Logger] Recorded Pressure: " .. string.format("%.2f", p) .. " atm")
end, logger)

-- 3. Start Logging
logger:setCycleTime(200)
logger:start()

print("\nSimulating Pressure Changes...")
for i = 1, 5 do
    if quasar.sleep then quasar.sleep(250) else os.execute("sleep 0.25") end
    pressure:setValue(1.0 + (i * 0.5))
end

-- 4. Stop Logging
print("Stopping Logger...")
logger:stop()

print("--- DataLogger Finished ---")
