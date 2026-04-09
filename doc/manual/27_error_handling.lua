-- 27_error_handling.lua
-- The Paramedic: Error Handling in Lua.
-- "Errors happen. It's how you recover from them that defines your script."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Error Handling (pcall) ---")

local root = n.createObject("Hospital")

-- 1. Create a method that is designed to fail
local riskyMethod = n.createLuaMethod("Surgery", function(owner, args)
    print("[Doctor] Starting surgery...")
    
    -- Triggering a Lua error intentionally
    error("Oops, dropped the scalpel into the NamedTree!")
    
    return nil
end, root)

-- 2. Execute normally (this would crash the script if unhandled)
-- To prevent crashing, we use Lua's built-in protected call (pcall)
print("\nAttempting risky surgery...")

-- We wrap the execution in a pcall to catch Lua exceptions
local status, errOrResult = pcall(function()
    return riskyMethod:execute(nil)
end)

if not status then
    print("[Paramedic] Caught an exception! Patient survived the crash.")
    print("Error Details: " .. tostring(errOrResult))
else
    print("[Paramedic] Surgery successful! (Wait, that wasn't supposed to happen...)")
end

-- 3. Handling invalid type casts
print("\nAttempting an invalid type cast...")
local temp = n.createDouble("Temp", 37.0, root)

-- Trying to cast a Double to a String (returns nil in Quasar instead of crashing)
local asString = temp:asString()

if not asString then
    print("[Paramedic] Safe casting prevented a crash! asString() returned nil for a NamedDouble.")
else
    print("Wait, how did that work?")
end

print("--- Error Handling Finished ---")
-- "Always wrap your risky bits in a pcall."
