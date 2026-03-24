local zmq = quasar.zmq
local n = quasar.named

print("ZMQ Test started")

local ctx = zmq.Context.new()
local pub = ctx:socket(zmq.PUB)
local sub = ctx:socket(zmq.SUB)

pub:bind("tcp://127.0.0.1:5555")
sub:connect("tcp://127.0.0.1:5555")
sub:subscribe("")

if quasar and quasar.sleep then
    quasar.sleep(100)
end

local root = n.createObject("root")
local valNode = n.createLong("val", 123, root)
local bufNode = n.createBuffer("buf", 4, root)
bufNode:write(0, {0xDE, 0xAD, 0xBE, 0xEF})

print("Publishing tree...")
pub:publishTree("topic", root)

print("Receiving tree...")
local restored = sub:receiveTree()

print("Restored tree name: " .. restored:getName())
if restored:getName() ~= "root" then
    error("Expected root, got " .. restored:getName())
end

local children = restored:getChildren()
print("Restored children count: " .. #children)
if #children ~= 2 then
    error("Expected 2 children, got " .. #children)
end

local child1 = children[1]
local longProxy = child1:asLong()
local rVal = longProxy:value()
print("Restored value: " .. tostring(rVal))
if tonumber(tostring(rVal)) ~= 123 then
    error("Expected 123, got " .. tostring(rVal))
end

print("ZMQ Test PASSED")
