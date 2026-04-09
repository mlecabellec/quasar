-- 13_sfc_simple.lua
-- The Assembly Line: Sequential Function Charts.
-- "A sequence is just a state machine that only knows how to move forward.
-- It's very determined."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Lua SFC Simulation ---")

-- 1. Setup the SFC Container
local sfc = n.createObject("AssemblyLineSFC")

-- 2. SFC Variables
local currentStep = n.createLong("CurrentStep", 0, sfc)
local conditionMet = n.createBoolean("TransitionCondition", false, sfc)

-- 3. Define Steps and Actions
-- "0: Idle, 1: Filling, 2: Capping, 3: Sealing, 4: Done"
local steps = {
    [0] = { name = "Idle", action = function() print("[SFC] Idle. Waiting for trigger...") end },
    [1] = { name = "Filling", action = function() print("[SFC] Filling the bottle... gurgle gurgle.") end },
    [2] = { name = "Capping", action = function() print("[SFC] Capping the bottle... squeak.") end },
    [3] = { name = "Sealing", action = function() print("[SFC] Sealing... zzzzt.") end },
    [4] = { name = "Done", action = function() print("[SFC] Bottle finished. Next!") end }
}

-- 4. Define the Engine Tick
local engineTick = n.createLuaMethod("Tick", function(owner, args)
    local stepObj = owner:getChild("CurrentStep"):asLong()
    local condObj = owner:getChild("TransitionCondition"):asBoolean()
    
    local step = stepObj:value()
    local cond = condObj:value()
    
    -- Execute current action
    if steps[step] then
        steps[step].action()
    end
    
    -- Check transition
    if cond then
        if step < 4 then
            print("[Transition] Moving from " .. steps[step].name .. " to " .. steps[step+1].name)
            stepObj:setValue(step + 1)
        else
            print("[Transition] Resetting sequence.")
            stepObj:setValue(0)
        end
        -- Reset condition after transition (Edge trigger logic)
        condObj:setValue(false)
    end
    
    return nil
end, sfc)

-- 5. Run the Sequence
print("Starting Assembly Line:")

engineTick:execute(nil) -- Tick 1 (Idle)

-- Trigger step 0 -> 1
conditionMet:setValue(true)
engineTick:execute(nil) -- Tick 2 (Transition to Filling)
engineTick:execute(nil) -- Tick 3 (Filling action)

-- Trigger step 1 -> 2
conditionMet:setValue(true)
engineTick:execute(nil) -- Tick 4 (Transition to Capping)
engineTick:execute(nil) -- Tick 5 (Capping action)

-- Trigger step 2 -> 3
conditionMet:setValue(true)
engineTick:execute(nil) -- Tick 6 (Transition to Sealing)
engineTick:execute(nil) -- Tick 7 (Sealing action)

-- Trigger step 3 -> 4
conditionMet:setValue(true)
engineTick:execute(nil) -- Tick 8 (Transition to Done)
engineTick:execute(nil) -- Tick 9 (Done action)

-- Trigger reset
conditionMet:setValue(true)
engineTick:execute(nil) -- Tick 10 (Reset)

print("\n--- SFC Finished ---")
-- "Production efficiency is up 42%. Management is pleased."
