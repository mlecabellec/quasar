-- Full Lua script example for NamedService
-- Demonstrates: Creation, Child methods as hooks, starting and stopping.

-- Disable buffering
io.stdout:setvbuf("no")

print("--- Quasar NamedService Example ---")

-- 1. Create a root object to hold our service
local root = quasar.named.createObject("root", nil)

-- 2. Create the NamedService
local service = quasar.named.createService("MyBackgroundSvc", root)

-- 3. Create a state variable (NamedLong) to track progress
local counter = quasar.named.createLong("counter", 0, service)

-- 4. Define the Hooks
-- Hook: onStart
quasar.named.createLuaMethod("onStart", function(owner, args)
    if owner == nil then print("[onStart] ERROR: owner is nil") return end
    print("[onStart] Service type: " .. owner:getType())
    print("[onStart] Service name: " .. owner:getName())
    local c = owner:getChild("counter"):asLong()
    c:setValue(100) -- Initialize with 100
end, service)

-- Hook: run (the main loop)
quasar.named.createLuaMethod("run", function(owner, args)
    if owner == nil then return end
    local c = owner:getChild("counter"):asLong()
    local val = c:value() + 1
    c:setValue(val)
    print("[run] Iteration: " .. val)
end, service)

-- Hook: onStop
quasar.named.createLuaMethod("onStop", function(owner, args)
    if owner == nil then return end
    print("[onStop] Service is shutting down. Final count: " .. owner:getChild("counter"):asLong():value())
end, service)

-- 5. Configure Cycle Time (in milliseconds)
service:setCycleTime(500) -- Run every 500ms

-- 6. Start the Service
print("Starting service...")
service:start()

-- 7. Wait for a few iterations in the main script thread
local startTime = os.time()
local duration = 5 -- seconds
print("Main script waiting for " .. duration .. " seconds...")

while os.difftime(os.time(), startTime) < duration do
    -- Yield CPU to let background thread work
    os.execute("sleep 0.1")
end

-- 8. Stop the Service
print("Stopping service...")
service:stop()

-- Wait a bit for the thread to actually finish
os.execute("sleep 0.5")

print("--- Example Finished ---")
