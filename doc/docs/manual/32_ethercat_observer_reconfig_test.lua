io.stdout:setvbuf("no")
print("--- EtherCAT Master Phases 3 & 4 Test ---")

local ok, resoem = pcall(function() return quasar.resoem end)
if not ok or not resoem then
    print("Failed to load resoem plugin.")
    os.exit(1)
end

local root = quasar.named.createObject("root", nil)
local service = resoem.EthercatMasterService.create("EcatMaster", root)

-- 1. Setup Master-level Observer
local masterObs = quasar.createObserver(function(event)
    print("[OBS-MASTER] Event received: " .. event:getName())
    if event:getName() == "SlaveStateChanged" then
        print("  Slave: " .. tostring(event:getChild("slaveIndex"):asLong():value()))
        print("  New State: " .. tostring(event:getChild("newState"):asLong():value()))
    end
end)
service:subscribe(masterObs)

print("Starting service...")
service:setInterface("lo")
service:start()

-- 2. Setup Slave-level Observer (if slaves discovered)
local slavesNode = service:getChild("slaves")
local slaves = slavesNode:getChildren()
if #slaves > 0 then
    print("Subscribing to slave_0...")
    local slave0 = slaves[1]
    local slaveObs = quasar.createObserver(function(event)
        print("[OBS-SLAVE0] Event received: " .. event:getName())
    end)
    slave0:subscribe(slaveObs)
end

-- 3. Execute Reconfigure (Demonstration)
print("Executing reconfigureSlave for index 0...")
local reconfigHook = service:getChild("reconfigureSlave")
if reconfigHook then
    local args = quasar.named.createObject("args")
    quasar.named.createLong("slaveIndex", 0, args)
    
    -- This will fail in 'lo' mode due to lack of real hardware but tests the logic flow
    local res = reconfigHook:asMethod():execute(args)
    if res then
        print("Reconfigure Success: " .. tostring(res:getChild("success"):asBoolean():value()))
    end
end

-- Wait a bit for async notifications
print("Waiting for any async notifications...")
quasar.sleep(500)

print("Stopping service...")
service:stop()
print("--- Test Finished ---")
