-- zmq_consumer.lua
-- Subscribes to a NamedBuffer tree via ZMQ SUB socket.

local ctx = quasar.zmq.Context()
local sub = ctx:socket(quasar.zmq.SUB)

sub:connect("tcp://localhost:5555")
sub:subscribe("mytopic")

print("Consumer: Connected to tcp://localhost:5555. Waiting for data on 'mytopic'...")

local receiveCount = 0

while true do
    -- [TSK-20260311-004.3.2] receiveTree is non-blocking.
    -- We use a loop with quasar.sleep to avoid busy-waiting while allowing other threads to run.
    local tree = sub:receiveTree()
    if tree ~= nil then
        receiveCount = receiveCount + 1
        
        -- Resolve the buffer node
        local bufNode = quasar.resolve(tree, "sensor_data")
        if bufNode ~= nil then
            local buf = bufNode:asBuffer()
            if buf ~= nil then
                local data = buf:read(0, 5)
                print("Consumer: Received tree #" .. tostring(receiveCount) .. 
                      " | sensor_data[4] = " .. tostring(data[5]))
            end
        end
    else
        -- [CS-0010.46] Yield engine lock to let other background services work.
        quasar.sleep(10)
    end
end
