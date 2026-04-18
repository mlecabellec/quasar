io.stdout:setvbuf("no")
print("--- EtherCAT Master Service Test ---")

-- The plugin must be loaded via command line: --plugin ./build/lib/libquasar_resoem_plugin.so
-- We use pcall to safely check if resoem exists
local ok, resoem = pcall(function() return quasar.resoem end)
if not ok or not resoem then
    print("Failed to load resoem plugin. Make sure to run with --plugin")
    os.exit(1)
end

local root = quasar.named.createObject("root", nil)
local service = resoem.EthercatMasterService.create("EcatMaster", root)

print("Service created: " .. service:getName())
service:setInterface("lo") -- Using loopback for testing to prevent failures

print("Starting service...")
service:start()

local slaves = service:getChild("slaves")
print("Slaves node present: " .. tostring(slaves ~= nil))

print("Executing refreshStatus...")
local refreshHook = service:getChild("refreshStatus")
if refreshHook then
    local res = refreshHook:asMethod():execute(nil)
    if res then
        print("Refresh Success: " .. tostring(res:getChild("success"):asBoolean():value()))
    end
end

print("Stopping service...")
service:stop()
print("--- Test Finished ---")
