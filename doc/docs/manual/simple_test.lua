-- Basic test for NamedLuaMethod in sre
print("--- Simple NamedLuaMethod Test ---")

local root = quasar.named.createObject("root", nil)
local counter = quasar.named.createLong("counter", 42, root)

local method = quasar.named.createLuaMethod("testMethod", function(owner, args)
    print("Inside Lua Method!")
    local c = owner:getChild("counter"):asLong()
    print("Current counter: " .. c:value())
    c:setValue(c:value() + 1)
    return nil
end, root)

print("Executing method...")
method:execute(nil)
print("New counter value: " .. counter:value())
print("--- Test Finished ---")
