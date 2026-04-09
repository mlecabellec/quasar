-- 12_state_machine_simple.lua
-- The Turing Machine: Finite State Automata.
-- "A machine that only knows a few things, but knows them very well."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Lua State Machine ---")

-- 1. Setup the FSM Container
local fsm = n.createObject("TrafficLightFSM")

-- 2. State Enum and Current State
-- "0: Red, 1: Green, 2: Yellow. Don't go on Yellow, it's a trap."
local RED = 0
local GREEN = 1
local YELLOW = 2

local currentState = n.createLong("CurrentState", RED, fsm)
local timer = n.createLong("Timer", 0, fsm)

-- 3. Define the Transition Logic
local fsmStep = n.createLuaMethod("Step", function(owner, args)
    local stateObj = owner:getChild("CurrentState"):asLong()
    local timeObj = owner:getChild("Timer"):asLong()
    
    local state = stateObj:value()
    local t = timeObj:value()
    
    t = t + 1 -- Increment timer
    timeObj:setValue(t)
    
    if state == RED then
        if t >= 5 then -- Stay red for 5 steps
            stateObj:setValue(GREEN)
            timeObj:setValue(0)
            print("[FSM] Red -> Green. Go!")
        else
            print("[FSM] Red... wait (" .. t .. "/5)")
        end
    elseif state == GREEN then
        if t >= 4 then -- Stay green for 4 steps
            stateObj:setValue(YELLOW)
            timeObj:setValue(0)
            print("[FSM] Green -> Yellow. Slow down!")
        else
            print("[FSM] Green... go (" .. t .. "/4)")
        end
    elseif state == YELLOW then
        if t >= 2 then -- Stay yellow for 2 steps
            stateObj:setValue(RED)
            timeObj:setValue(0)
            print("[FSM] Yellow -> Red. Stop!")
        else
            print("[FSM] Yellow... brake (" .. t .. "/2)")
        end
    end
    
    return nil
end, fsm)

-- 4. Run the FSM
print("Starting Traffic Light Sequence:")
for i = 1, 12 do
    print("\n--- Step " .. i .. " ---")
    fsmStep:execute(nil)
end

print("\n--- FSM Finished ---")
-- "And so the endless cycle continues. Such is the life of a traffic light."
