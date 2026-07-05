-- 14_cause_effect_matrix.lua
-- The Bureaucracy: Cause and Effect Matrix.
-- "If A happens, do B. If C happens, do D and E. It's just paperwork, really."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Lua Cause & Effect Matrix ---")

-- 1. Setup Matrix Container
local cem = n.createObject("SafetyMatrix")

-- 2. Inputs (Causes)
local alarmHighTemp = n.createBoolean("AlarmHighTemp", false, cem)
local alarmLowPress = n.createBoolean("AlarmLowPress", false, cem)
local manualOverride = n.createBoolean("ManualOverride", false, cem)

-- 3. Outputs (Effects)
local shutdownReactor = n.createBoolean("ShutdownReactor", false, cem)
local ventGas = n.createBoolean("VentGas", false, cem)
local soundSiren = n.createBoolean("SoundSiren", false, cem)

-- 4. The Matrix (Rules)
-- Row = Cause, Column = Effect
-- X means if Cause is TRUE, Effect must be TRUE.
local rules = {
    -- Format: {Cause, {Effect1, Effect2, ...}}
    {"AlarmHighTemp",  {"ShutdownReactor", "VentGas", "SoundSiren"}},
    {"AlarmLowPress",  {"ShutdownReactor", "SoundSiren"}},
    {"ManualOverride", {"ShutdownReactor"}}
}

-- 5. Evaluate Method
local evalCEM = n.createLuaMethod("Evaluate", function(owner, args)
    -- Reset all effects initially (or keep them latched based on design)
    owner:getChild("ShutdownReactor"):asBoolean():setValue(false)
    owner:getChild("VentGas"):asBoolean():setValue(false)
    owner:getChild("SoundSiren"):asBoolean():setValue(false)
    
    local anyCause = false
    
    for i, rule in ipairs(rules) do
        local causeName = rule[1]
        local effects = rule[2]
        
        local causeState = owner:getChild(causeName):asBoolean():value()
        if causeState then
            print("[CEM] Triggered Cause: " .. causeName)
            anyCause = true
            -- Apply effects
            for j, effectName in ipairs(effects) do
                owner:getChild(effectName):asBoolean():setValue(true)
                print("      -> Triggering Effect: " .. effectName)
            end
        end
    end
    
    if not anyCause then
        print("[CEM] All clear. Normal operation.")
    end
    return nil
end, cem)

-- 6. Test Scenarios
print("\nScenario 1: Normal Operation")
evalCEM:execute(nil)

print("\nScenario 2: Manual Override Activated")
manualOverride:setValue(true)
evalCEM:execute(nil)

print("\nScenario 3: Catastrophic Failure (High Temp + Low Pressure)")
manualOverride:setValue(false)
alarmHighTemp:setValue(true)
alarmLowPress:setValue(true)
evalCEM:execute(nil)

print("\n--- CEM Finished ---")
-- "Bureaucracy is expanding to meet the needs of the expanding bureaucracy."
