-- 18_opcua_server.lua
-- The Bureaucrat: OPC UA Server.
-- "Exposing your data to the world, securely, and with an unreasonable amount of overhead.
-- Ah, industry standards."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: OPC UA Server ---")
if not quasar.opcua then
    print("ERROR: OPC UA plugin not loaded. Run with '--plugin build/lib/quasar_opcua_plugin.so'")
    os.exit(1)
end

-- 1. Setup the Local Tree (This is what we will expose)
local root = n.createObject("OpcUaServerNode")
local machine = n.createObject("CNC_Machine", root)
local xPos = n.createDouble("X_Position", 0.0, machine)
local yPos = n.createDouble("Y_Position", 0.0, machine)
local running = n.createBoolean("IsRunning", true, machine)

-- 2. Create the OPC UA Server Service
print("Initializing Server Service...")
local server = quasar.opcua.createServer("MyUAServer", root)
server:setPort(4841)

-- 3. Define a Method to expose
-- Methods in the tree are automatically exported as OPC UA Methods!
n.createLuaMethod("HomeAxes", function(owner, args)
    print("[Server] HomeAxes method called via OPC UA!")
    owner:getChild("X_Position"):asDouble():setValue(0.0)
    owner:getChild("Y_Position"):asDouble():setValue(0.0)
    return n.createString("Response", "Axes homed successfully.")
end, machine)

-- 4. Start the Server
print("Starting OPC UA Server on port 4841...")
server:start()

-- 5. Simulate Data Changes in the background
print("Simulating machine movement for 5 seconds...")
for i = 1, 5 do
    if quasar.sleep then quasar.sleep(1000) else os.execute("sleep 1") end
    local x = xPos:value()
    xPos:setValue(x + 10.5)
    print("  -> Updated X_Position to: " .. xPos:value())
end

-- 6. Cleanup
print("Stopping Server...")
server:stop()

print("--- OPC UA Server Finished ---")
