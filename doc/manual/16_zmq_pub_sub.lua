-- 16_zmq_pub_sub.lua
-- The Town Crier: ZMQ PUB/SUB
-- "Hear ye, hear ye! A tree has been updated!"

io.stdout:setvbuf("no")

local zmq = quasar.zmq
local n = quasar.named

print("--- Quasar: ZMQ PUB/SUB Demonstration ---")
if not zmq then
    print("ERROR: ZMQ plugin not loaded. Run with '--plugin build/lib/quasar_zmq.so'")
    os.exit(1)
end

-- 1. Setup the Network
local ctx = zmq.Context.new()
local pub = ctx:socket(zmq.PUB)
local sub = ctx:socket(zmq.SUB)

print("Binding Publisher to port 5556...")
pub:bind("tcp://127.0.0.1:5556")

print("Connecting Subscriber to port 5556...")
sub:connect("tcp://127.0.0.1:5556")
sub:subscribe("QuasarUpdates") -- Subscribe to the topic

-- Wait for connection to establish
if quasar.sleep then quasar.sleep(100) else os.execute("sleep 0.1") end

-- 2. Setup the Data
local root = n.createObject("WeatherStation")
local temp = n.createDouble("Temperature", 22.5, root)
local hum = n.createDouble("Humidity", 45.0, root)

-- 3. Publish the Tree
print("Publishing WeatherStation data under topic 'QuasarUpdates'...")
-- Update values
temp:setValue(23.1)
hum:setValue(48.2)

-- publishTree sends the whole hierarchy
pub:publishTree("QuasarUpdates", root)

-- 4. Receive the Tree
print("Subscriber waiting for tree...")
local restoredRoot = sub:receiveTree()

if restoredRoot then
    print("Successfully received tree: " .. restoredRoot:getName())
    
    local rTemp = restoredRoot:getChild("Temperature")
    local rHum = restoredRoot:getChild("Humidity")
    
    if rTemp and rHum then
        print("Restored Temperature: " .. rTemp:asDouble():value() .. " C")
        print("Restored Humidity: " .. rHum:asDouble():value() .. " %")
    else
        print("Missing children in restored tree.")
    end
else
    print("FAILED to receive tree within timeout.")
end

print("--- ZMQ PUB/SUB Finished ---")
