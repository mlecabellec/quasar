-- zmq_producer.lua
-- Publishes a NamedBuffer tree at 10Hz using ZMQ PUB socket.

-- Initialize ZMQ Context and PUB socket
local ctx = quasar.zmq.Context()
local pub = ctx:socket(quasar.zmq.PUB)
pub:bind("tcp://*:5555")

print("Producer: Bound to tcp://*:5555. Starting publisher...")

-- Create a data tree
local root = quasar.named.createObject("telemetry")
local buf = quasar.named.createBuffer("sensor_data", 1024, root)

local counter = 0

-- Publish at 10Hz
while true do
    -- Update buffer with some dummy payload
    local data = { 0xDE, 0xAD, 0xBE, 0xEF, counter % 256 }
    buf:write(0, data)
    
    -- Publish the entire tree under 'mytopic' topic
    pub:publishTree("mytopic", root)
    
    counter = counter + 1
    if counter % 10 == 0 then
        print("Producer: Published " .. tostring(counter) .. " trees.")
    end
    
    quasar.os.sleep(100) -- Sleep ~100ms
end
