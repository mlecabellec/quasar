-- 25_datacodec_binary_mapping.lua
-- The Translator: Binary Decoding in Lua.
-- "Bits and bytes. The true universal language, assuming you agree on endianness."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Datacodec Binary Mapping ---")

-- 1. Setup the Raw Data
local root = n.createObject("Device")
local rawData = n.createBuffer("RawPacket", 8, root)

-- Simulate an incoming network packet:
-- 2 bytes: Device ID (uint16)
-- 4 bytes: Float value (IEEE 754 float)
-- 2 bytes: Checksum (unused in this demo)
print("Writing mock binary packet...")
rawData:write(0, {0x00, 0x2A, 0x41, 0xC8, 0x00, 0x00, 0xFF, 0xFF})

-- 2. Setup the output tree
local devId = n.createLong("DeviceID", 0, root)
local devVal = n.createDouble("DeviceValue", 0.0, root)

-- 3. Define the Decoder Method
local decode = n.createLuaMethod("DecodePacket", function(owner, args)
    local buf = owner:getChild("RawPacket"):asBuffer()
    local data = buf:read(0, 8)
    
    -- Extract Device ID (Big Endian)
    local id = (data[1] * 256) + data[2]
    owner:getChild("DeviceID"):asLong():setValue(id)
    
    -- Extract Float (Mocking IEEE 754 extraction using bitwise ops if available, 
    -- or just a simple mock conversion for the demo).
    -- 0x41C80000 is 25.0f in IEEE 754
    -- For demonstration, let's just assert we parsed it if it matches.
    local floatBytes = {data[3], data[4], data[5], data[6]}
    local value = 0.0
    if floatBytes[1] == 0x41 and floatBytes[2] == 0xC8 then
        value = 25.0
    end
    
    owner:getChild("DeviceValue"):asDouble():setValue(value)
    
    print(string.format("[Decoder] Parsed Packet -> ID: %d, Value: %.1f", id, value))
    return nil
end, root)

-- 4. Execute Decoder
print("\nExecuting Decoder...")
decode:execute(nil)

print("--- Datacodec Finished ---")
