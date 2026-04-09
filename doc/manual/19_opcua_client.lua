-- 19_opcua_client.lua
-- The Auditor: OPC UA Client.
-- "Asking politely for data using a standardized language."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: OPC UA Client ---")
if not quasar.opcua then
    print("ERROR: OPC UA plugin not loaded. Run with '--plugin build/lib/quasar_opcua_plugin.so'")
    os.exit(1)
end

-- 1. Create a root to hold our client service
local root = n.createObject("ClientRoot")

-- 2. Create the Client Service
local client = quasar.opcua.createClient("MyUAClient", root)

-- 3. Configure the connection
-- "Let's connect to the server from script 18."
print("Configuring Client to connect to opc.tcp://localhost:4841")
client:setUrl("opc.tcp://localhost:4841")

-- 4. Start the Client
-- This creates a local mirror of the server's tree as children of the client object.
print("Starting Client...")
client:start()

-- Give it some time to connect and browse the tree.
print("Waiting for connection and browse... (2 seconds)")
if quasar.sleep then quasar.sleep(2000) else os.execute("sleep 2") end

-- 5. Interacting with the mirrored tree
-- Quasar automatically maps OPC UA variables to NamedObjects.
print("Browsing local mirror...")
local serverRoot = client:getChild("MyUAServer")
if serverRoot then
    print("Successfully found mirrored root: " .. serverRoot:getName())
    
    -- Resolving a path relative to the mirrored root
    local xPosNode = quasar.resolve(serverRoot, "CNC_Machine/X_Position")
    if xPosNode then
        print("Remote X_Position read: " .. xPosNode:asDouble():value())
    else
        print("FAILED to find X_Position.")
    end

    -- Calling a remote method
    local methodNode = quasar.resolve(serverRoot, "CNC_Machine/HomeAxes")
    if methodNode then
        print("Calling remote method 'HomeAxes'...")
        local result = methodNode:asMethod():execute(nil)
        
        if result and result:asString() then
            print("Remote method replied: '" .. result:asString():value() .. "'")
        else
            print("Remote method call failed or returned nil.")
        end
    else
        print("FAILED to find remote 'HomeAxes' method.")
    end
else
    print("FAILED to find server root (MyUAServer). Is the server script running?")
end

-- 6. Cleanup
print("Stopping Client...")
client:stop()

print("--- OPC UA Client Finished ---")
