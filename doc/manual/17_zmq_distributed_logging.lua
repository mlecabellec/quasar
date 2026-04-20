-- 17_zmq_distributed_logging.lua
-- The Gossiper: Distributed Logging via ZMQ.
-- "Why write to a file when you can shout your logs into the void of the network?"

io.stdout:setvbuf("no")

local zmq = quasar.zmq
local n = quasar.named

print("--- Quasar: ZMQ Distributed Logging ---")
if not zmq then
    print("ERROR: ZMQ plugin not loaded.")
    os.exit(1)
end

local ctx = zmq.Context.new()
local pub = ctx:socket(zmq.PUB)
local sub = ctx:socket(zmq.SUB)

pub:bind("tcp://*:5557")
sub:connect("tcp://127.0.0.1:5557")

-- Subscriber can filter by topic (e.g., only "ERROR" or "INFO")
-- Subscribing to "" gets everything. Subscribing to "ERROR" gets only errors.
sub:subscribe("LOG:ERROR") 
sub:subscribe("LOG:WARN")

print("Waiting for ZMQ connections to stabilize...")
if quasar.sleep then quasar.sleep(1000) else os.execute("sleep 1") end

local logRoot = n.createObject("LogEntry")
local msgNode = n.createString("Message", "", logRoot)
local codeNode = n.createLong("Code", 0, logRoot)

local function broadcastLog(level, msg, code)
    msgNode:setValue(msg)
    codeNode:setValue(code)
    local topic = "LOG:" .. level
    print("Broadcasting -> " .. topic .. " | " .. msg)
    pub:publishTree(topic, logRoot)
end

-- Broadcast some logs
broadcastLog("INFO", "System startup sequence initiated.", 0)
broadcastLog("WARN", "Temperature slightly elevated.", 101)
broadcastLog("INFO", "Running diagnostics...", 0)
broadcastLog("ERROR", "Reactor containment breach detected!", 500)

-- Process received logs
print("\nSubscriber listening (filtering for WARN and ERROR only)...")
-- We expect to receive exactly 2 messages (WARN and ERROR)
for i = 1, 2 do
    local entry = sub:receiveTree()
    if entry then
        local rMsg = entry:getChild("Message"):asString():value()
        local rCode = entry:getChild("Code"):asLong():value()
        print("Received Log -> Code: " .. rCode .. ", Message: " .. rMsg)
    end
end

print("--- Distributed Logging Finished ---")
