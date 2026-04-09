-- 08_named_service_lifecycle.lua
-- The Worker Bee: Services and Background Loops.
-- "Services are like background actors in a movie. You don't always see them,
-- but the world would be very quiet without them."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: NamedService Lifecycle ---")

-- 1. Create a container
local root = n.createObject("TheHive")

-- 2. Create the Service
-- "A service is a NamedObject with its own heart (a thread) and its own agenda."
local worker = n.createService("BusyBee", root)
local honey = n.createLong("HoneyCount", 0, worker)

-- 3. Define Lifecycle Hooks
-- 'onStart' runs once when the service thread starts.
n.createLuaMethod("onStart", function(owner, args)
    print("[onStart] Bee is waking up. Cleaning wings...")
    owner:getChild("HoneyCount"):asLong():setValue(100)
end, worker)

-- 'run' runs repeatedly based on cycle time.
-- "Gathering honey. One byte at a time. It's not much, but it's honest work."
n.createLuaMethod("run", function(owner, args)
    local h = owner:getChild("HoneyCount"):asLong()
    h:setValue(h:value() + 1)
    print("[run] Honey gathering... count is " .. h:value())
end, worker)

-- 'onStop' runs once when the service is told to stop.
n.createLuaMethod("onStop", function(owner, args)
    print("[onStop] Sun is setting. Bee is going back to the Hive.")
end, worker)

-- 4. Configure and Engage
print("Setting cycle time to 500ms...")
worker:setCycleTime(500)

print("Engaging service...")
worker:start()

-- 5. Wait for the magic
print("Monitoring bee for 3 seconds...")
local start = os.time()
while os.difftime(os.time(), start) < 3 do
    os.execute("sleep 0.2")
end

-- 6. Disengage
print("Stopping service...")
worker:stop()

print("Final honey count: " .. honey:value())
print("--- Service Finished ---")
-- "Zzzzzzz. The worker rests. Until the next script."
