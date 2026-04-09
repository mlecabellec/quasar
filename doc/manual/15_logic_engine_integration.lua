-- 15_logic_engine_integration.lua
-- The Nervous System: Observers and Reactive Logic.
-- "Why ask 'Are we there yet?' when you can just wait to be told?"

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Reactive Logic Observers ---")

-- 1. Setup Data
local root = n.createObject("ReactorCore")
local tempSensor = n.createDouble("CoreTemp", 50.0, root)
local tempWarning = n.createBoolean("TempWarning", false, root)

-- 2. Define the Logic
local checkTemp = n.createLuaMethod("CheckTemperature", function(owner, args)
    local t = owner:getChild("CoreTemp"):asDouble():value()
    local w = owner:getChild("TempWarning"):asBoolean()
    
    if t > 100.0 then
        if not w:value() then
            print("[Reactor] Warning: Temperature exceeding critical limits (" .. t .. " C)!")
            w:setValue(true)
        end
    else
        if w:value() then
            print("[Reactor] Temperature returning to normal (" .. t .. " C).")
            w:setValue(false)
        end
    end
    return nil
end, root)

-- 3. Create an Observer
-- "Observers are like nosy neighbors. They watch everything you do."
if quasar.createObserver then
    print("Setting up observer on CoreTemp...")
    local obs = quasar.createObserver(function(obj)
        print("[Observer] CoreTemp changed! Re-evaluating logic...")
        checkTemp:execute(nil)
    end)

    -- Attach observer to the sensor. Sensor must be an ActiveEntity, which primitive types often act as,
    -- or we cast it to ActiveEntity if it supports the interface.
    local activeTemp = tempSensor:asActive()
    if activeTemp then
        activeTemp:subscribe(obs)
        print("Observer subscribed successfully.")
    else
        print("CoreTemp does not support ActiveEntity subscription in this Lua version.")
        print("Manually evaluating for demonstration:")
    end
else
    print("quasar.createObserver not available. Using manual evaluation.")
end

-- 4. Test the Integration
print("\n[Simulation] Heating up reactor...")
for i = 60, 120, 20 do
    print("\n   -> Setting temp to " .. i)
    tempSensor:setValue(i)
    
    -- [CS-0010.46] Use quasar.sleep to allow observers/background tasks to run.
    if quasar.sleep then quasar.sleep(100) end

    -- If observers aren't natively supported on primitives in this build,
    -- we manually execute the logic hook.
    if not tempSensor:asActive() then checkTemp:execute(nil) end
end

print("\n[Simulation] Cooling down reactor...")
tempSensor:setValue(90)
if quasar.sleep then quasar.sleep(100) end
if not tempSensor:asActive() then checkTemp:execute(nil) end

print("\n--- Reactive Logic Finished ---")
-- "The reactor is safe. The observer, however, is traumatized."
