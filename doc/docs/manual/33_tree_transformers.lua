-- 33_tree_transformers.lua
-- The Grand Morph: Tree Transformers.
-- "More than meets the eye! Trees that change their shape faster than a Cybertronian."

io.stdout:setvbuf("no")

local n = quasar.named
local t = quasar.named.traversal

print("--- Quasar: Tree Transformers ---")

-- 1. Setup our "Before" Tree: A scrapyard of robot parts.
-- "In the scrapyard, everything is just a generic object. We need to scan and identify them."
local scrapyard = n.createObject("Scrapyard")
local bin1 = n.createObject("Bin1", scrapyard)
local item1 = n.createObject("RustyPart", bin1)
local item2 = n.createObject("MetalScrap", bin1)

local bin2 = n.createObject("Bin2", scrapyard)
local item3 = n.createObject("OldCircuit", bin2)

print("Scrapyard topology before transformation:")
print(n.serialization.toYaml(scrapyard))

-- 2. Define a Transformer
-- "The Transformer is our scanning and assembly machine."
local scanner = t.Transformer.new()

-- 3. Add a simple Rule: Identify "RustyPart" and upgrade it to a "FusionCore"
-- Rules have a Predicate (the 'if') and a Generator (the 'then').
print("Adding upgrade rule...")
scanner:addRule(
    function(ctx) -- Predicate: Who are you?
        return ctx:getNode():getName() == "RustyPart"
    end,
    function(ctx, trans) -- Generator: What do you become?
        print("[Scanner] Found RustyPart! Upgrading to FusionCore...")
        local core = n.createLong("FusionCore", 9000)
        return core
    end,
    10 -- Priority: I'm more important than generic scrap.
)

-- 4. Add a destructive Rule: Get rid of "MetalScrap"
-- "Returning an empty table {} or nil effectively deletes the node."
print("Adding recycling rule...")
scanner:addRule(
    function(ctx)
        return ctx:getNode():getName() == "MetalScrap"
    end,
    function(ctx, trans)
        print("[Scanner] Recycling MetalScrap. Beep boop.")
        return {} -- Goodbye!
    end,
    5
)

-- 5. Out-of-Place Transformation
-- "Let's create a NEW, clean Laboratory from our Scrapyard."
print("\nExecuting Out-of-Place Transformation...")
local lab_roots = scanner:transform(scrapyard)
local laboratory = lab_roots[1]

print("\nLaboratory topology after upgrade:")
print(n.serialization.toYaml(laboratory))

-- 6. Predefined Rules: Slicing and Dicing
-- "Sometimes we have a block of memory (a buffer) that we want to turn into a structure."
print("\n--- Advanced: Predefined Rules ---")

local dataStream = n.createBuffer("TelemetryBuffer", 12)
dataStream:write(0, {0x00, 0x00, 0x00, 0x2A, 0x41, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) -- 42 and 25.0

local mapper = t.Transformer.new()

-- Use a predefined 'castToStructure' rule
-- It maps offsets in a buffer to bound primitives (Zero-Copy!)
local mapping = {
    { name = "id",      type = "int32",   offset = 0, endian = quasar.Endianness.BigEndian },
    { name = "voltage", type = "float32", offset = 4, endian = quasar.Endianness.BigEndian }
}
mapper:addRule(t.rules.castToStructure("TelemetryBuffer", mapping))

local mapped_roots = mapper:transform(dataStream)
local robotState = mapped_roots[1]

print("Mapped structure (Zero-Copy):")
print(n.serialization.toYaml(robotState))

-- 7. Verification of Live View
-- "Because it's a live view (Zero-Copy), changing the buffer changes the structural objects!"
local vNode = quasar.resolve(robotState, "voltage"):asFloat()
print("Initial voltage: " .. vNode:value())

print("Updating raw buffer...")
dataStream:write(4, {0x42, 0x48, 0x00, 0x00}) -- 50.0 in float64 BE? Wait, 0x42480000 is 50.0f
-- Actually let's just check it changed.
print("Updated voltage: " .. vNode:value())

print("--- Transformers Finished ---")
-- "Autobots, roll out! (And don't forget to garbage collect)."
