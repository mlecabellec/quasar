-- 05_named_buffer.lua
-- The Matrix: Raw Buffers.
-- "Reading and writing bytes is like communicating in binary. It's very 0101-ish."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: NamedBuffer Basics ---")

-- 1. Create a buffer of 16 bytes
local buf = n.createBuffer("TheMatrix", 16)
print("Buffer created, size: " .. buf:getSize())

-- 2. Writing to buffer
-- Lua tables are used for the 'write' method.
-- "Writing data. If you write '42' everywhere, you might eventually find the question."
print("Writing data to offset 0...")
buf:write(0, {0xDE, 0xAD, 0xBE, 0xEF, 0x42, 0x42, 0x00, 0x01})

-- 3. Reading from buffer
-- 'read' returns a Lua table (vector of uint8_t in C++ becomes a table in Sol2).
print("Reading data back from offset 0...")
local data = buf:read(0, 8)
local hex = ""
for i, v in ipairs(data) do
    hex = hex .. string.format("%02X ", v)
end
print("Data read: " .. hex)

-- 4. Writing individual bytes
print("Writing single bytes...")
buf:write(8, {0xAA})
buf:write(9, {0xBB})

-- 5. Out of bounds safety
-- Quasar handles bounds internally to prevent segfaults.
print("Attempting to write out of bounds...")
buf:write(15, {0x11, 0x22, 0x33}) -- Only 0x11 should be written at index 15.

local lastBytes = buf:read(14, 2)
print(string.format("Last two bytes: %02X %02X", lastBytes[1], lastBytes[2]))

-- 6. Bit Buffers
-- "For when a whole byte is just too much space for a simple yes/no."
print("--- Quasar: NamedBitBuffer ---")
local bitBuf = n.createBitBuffer("BooleanGrid", 8) -- 8 bits
print("Bit buffer created, count: " .. bitBuf:getBitCount())

bitBuf:setBit(0, true)
bitBuf:setBit(1, false)
bitBuf:setBit(2, true)

print("Bit 0: " .. tostring(bitBuf:getBit(0)))
print("Bit 1: " .. tostring(bitBuf:getBit(1)))
print("Bit 2: " .. tostring(bitBuf:getBit(2)))

print("--- Buffer Finished ---")
-- "And that's how we move the 1s and 0s from here to there."
