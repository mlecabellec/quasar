-- Basic test for NamedLuaMethod in sre
io.stdout:setvbuf("no")
print("--- Explicit Hook Test ---")

local root = quasar.named.createObject("root", nil)
local service = quasar.named.createService("MySvc", root)
local counter = quasar.named.createLong("counter", 42, service)

local onStart = quasar.named.createLuaMethod("onStart", function(owner, args)
    print("Inside onStart!")
    if owner then print("Owner name: " .. owner:getName()) end
    return nil
end, service)

print("Calling getChild('onStart')...")
local hook = service:getChild("onStart")
if hook then
    print("Hook found, type: " .. hook:getType())
    local method = hook:asMethod()
    if method then
        print("Executing asMethod...")
        method:execute(nil)
    else
        print("FAILED to cast to method")
    end
else
    print("FAILED to find child 'onStart'")
end

print("--- Test Finished ---")
