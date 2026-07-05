-- 07_named_method_simple.lua
-- The Call of Duty: Methods and Hooks.
-- "In Quasar, data doesn't just sit there. It does things. Like a cat, but useful."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: NamedLuaMethod Basics ---")

-- 1. Setup Data
local root = n.createObject("Laboratory")
local counter = n.createLong("Samples", 0, root)

-- 2. Define a Lua Method
-- A Lua method receives (owner, args) and returns a NamedObject (or nil).
-- "Methods are like instructions for a computer. But with more 'please' and 'thank you'."
local addSample = n.createLuaMethod("AddSample", function(owner, args)
    local c = owner:getChild("Samples"):asLong()
    local oldVal = c:value()
    local newVal = oldVal + 1
    c:setValue(newVal)
    
    print("[Method] Sample added. Count is now: " .. newVal)
    
    -- We can return the new value as a NamedObject
    return n.createLong("Result", newVal)
end, root)

-- 3. Execute the Method
print("Calling AddSample method...")
local result = addSample:execute(nil) -- No arguments needed here

if result then
    print("Method returned: " .. result:asLong():value())
end

-- 4. Methods with Arguments
-- Arguments are passed as a single NamedObject (often a Map or Array for multiple args).
local addMultiple = n.createLuaMethod("AddMultiple", function(owner, args)
    local countToAdd = 1
    if args and args:asLong() then
        countToAdd = args:asLong():value()
    end
    
    local c = owner:getChild("Samples"):asLong()
    c:setValue(c:value() + countToAdd)
    print("[Method] Added " .. countToAdd .. " samples.")
    return nil
end, root)

print("Calling AddMultiple with argument 5...")
local arg = n.createLong("count", 5)
addMultiple:execute(arg)

print("Final samples count: " .. counter:value())

-- 5. Introspection
print("AddSample type: " .. addSample:getType())
local m = root:getChild("AddSample")
if m and m:asMethod() then
    print("Found method via child lookup. Executing again...")
    m:asMethod():execute(nil)
end

print("--- Methods Finished ---")
-- "And that's how we make the tree dance."
