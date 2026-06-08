-- 34_raw_ethernet_service.lua
-- TSK-20260529-001: Raw Ethernet Socket Service.
-- Demonstrates construction, property access, transformer rules attachment,
-- and graceful degradation when CAP_NET_RAW is unavailable.

io.stdout:setvbuf("no")

print("=== Quasar: RawEthernetService Lua Binding Test ===")

if not quasar or not quasar.net then
    print("ERROR: Net plugin not loaded. Run with '--plugin build/lib/quasar_net_plugin.so'")
    os.exit(1)
end

if not quasar.net.RawEthernetService then
    print("ERROR: RawEthernetService not exposed in quasar.net namespace.")
    os.exit(1)
end

print("[STEP 1] Creating RawEthernetService on 'lo' interface...")
local svc = quasar.net.RawEthernetService.new("rawEth")
assert(svc ~= nil, "RawEthernetService creation failed")
print("[OK] Name: " .. svc:getName() .. ", Type: " .. svc:getType())

print("\n[STEP 2] Verifying child property nodes...")
local ifName = svc.interfaceName
assert(ifName ~= nil, "interfaceName node missing")
print("[OK] interfaceName.value = " .. ifName:value())

local ethType = svc.etherType
assert(ethType ~= nil, "etherType node missing")
print("[OK] etherType.value = " .. tostring(ethType:value()))

local inFrame = svc.incomingFrame
assert(inFrame ~= nil, "incomingFrame node missing")
print("[OK] incomingFrame node type: " .. inFrame:getType())

local outFrame = svc.outgoingFrame
assert(outFrame ~= nil, "outgoingFrame node missing")
print("[OK] outgoingFrame node type: " .. outFrame:getType())

local inTree = svc.incomingTree
assert(inTree ~= nil, "incomingTree node missing")
print("[OK] incomingTree node type: " .. inTree:getType())

print("\n[STEP 3] Attaching a castToStructure transformer rule...")
if quasar.named and quasar.named.traversal and quasar.named.traversal.rules then
    local rules = quasar.named.traversal.rules
    local mappings = {
        { name = "destMacUpper", type = "int32", offset = 0 },
        { name = "destMacLower", type = "int32", offset = 2 },
        { name = "srcMacUpper",  type = "int32", offset = 4 },
        { name = "etherTypeField", type = "int32", offset = 8 },
    }
    local rule = rules.castToStructure("incomingFrame", mappings, 0)
    svc:addRule(rule)
    print("[OK] castToStructure rule attached.")
else
    print("[SKIP] quasar.named.traversal.rules not available in this context.")
end

print("\n[STEP 4] Attempting service start (expects CAP_NET_RAW or graceful fail)...")
local ok, err = pcall(function()
    svc:start()
end)

if ok then
    print("[OK] Service started. isRunning = " .. tostring(svc:isRunning()))
    print("     Waiting 200ms for poll loop...")
    if quasar.sleep then quasar.sleep(200) end
    
    print("[STEP 5] Attempting raw frame send (may fail if no route)...")
    local sendOk, sendErr = pcall(function()
        svc:send()
    end)
    if sendOk then
        print("[OK] send() dispatched without exception.")
    else
        print("[WARN] send() threw (no AF_PACKET route or empty frame): " .. tostring(sendErr))
    end
    
    print("[STEP 6] Stopping service...")
    svc:stop()
    print("[OK] Service stopped. isRunning = " .. tostring(svc:isRunning()))
else
    -- Graceful degradation: no CAP_NET_RAW in CI environment
    local errStr = tostring(err)
    if errStr:find("Permission denied") or errStr:find("Operation not permitted") then
        print("[SKIPPED] No CAP_NET_RAW capability. This is expected in CI/unprivileged environments.")
        print("  Details: " .. errStr)
    else
        print("[FAIL] Unexpected error during start(): " .. errStr)
        os.exit(1)
    end
end

print("\n=== RawEthernetService Binding Test PASSED ===")
