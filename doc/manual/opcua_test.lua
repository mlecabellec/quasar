-- opcua_test.lua
-- Requires --plugin ./build/lib/quasar_opcua_plugin.so

io.stdout:setvbuf("no")

local function log(msg)
    print("[Lua] " .. msg)
end

log("--- Starting OPC UA Test ---")

local root = quasar.named.createObject("root")
local serverSvc = quasar.opcua.createServer("OpcUaServer", root)
serverSvc:setPort(4840)

local dataNode = quasar.named.createObject("Data", serverSvc)
local myInt = quasar.named.createLong("MyInt", 42, dataNode)

local myMethod = quasar.named.createLuaMethod("Multiply", function(owner, args)
    log("Multiply called!")
    local input = args:asLong():value()
    local result = input * 2
    local resObj = quasar.named.createLong("result", result)
    return resObj
end, dataNode)

log("Starting Server...")
serverSvc:start()

-- Give it some time
os.execute("sleep 2")

log("Creating Client...")
local clientSvc = quasar.opcua.createClient("OpcUaClient")
clientSvc:setUrl("opc.tcp://localhost:4840")

log("Starting Client...")
clientSvc:start()

os.execute("sleep 2")

log("Browsing local mirror...")
local remoteRoot = clientSvc:getChild("OpcUaServer")
if remoteRoot then
    log("Found remote root: " .. remoteRoot:getName())
    local remoteInt = quasar.resolve(remoteRoot, "Data/MyInt")
    if remoteInt then
        log("Found remote MyInt, value: " .. tostring(remoteInt:asLong():value()))
    else
        log("FAILED to find remote MyInt")
    end

    log("Testing remote method call...")
    local remoteMethod = quasar.resolve(remoteRoot, "Data/Multiply")
    if remoteMethod then
        log("Found remote Multiply method")
        local args = quasar.named.createLong("input", 21)
        local result = remoteMethod:asMethod():execute(args)
        if result then
            log("Method call result: " .. tostring(result:asLong():value()))
        else
            log("FAILED to get method result")
        end
    else
        log("FAILED to find remote Multiply method")
    end
else

    log("FAILED to find remote root")
end

log("Stopping services...")
clientSvc:stop()
serverSvc:stop()

log("--- OPC UA Test Finished ---")
