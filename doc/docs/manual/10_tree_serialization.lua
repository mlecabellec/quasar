-- 10_tree_serialization.lua
-- The Grand Archivist: Saving and Loading.
-- "Writing down what you have is the best way to remember what you've lost.
-- Or just to copy your configuration between systems."

io.stdout:setvbuf("no")

local n = quasar.named
local s = quasar.named.serialization

print("--- Quasar: Serialization ---")

-- 1. Setup a tree to serialize
local root = n.createObject("TheArchive")
local config = n.createObject("Config", root)
local v1 = n.createLong("ID", 1234, config)
local v2 = n.createDouble("Pi", 3.14159, config)
local v3 = n.createString("Name", "Deep Thought", config)

-- 2. JSON Serialization
-- "JSON is like the lingua franca of data. Everybody speaks it, even if they're not happy about it."
print("Serializing to JSON...")
local json = s.toJson(root)
print("JSON Representation:\n" .. json)

-- 3. YAML Serialization
-- "YAML is like JSON but with more whitespace and fewer curly braces. 
-- It's sensitive, like a poet."
print("Serializing to YAML...")
local yaml = s.toYaml(root)
print("YAML Representation:\n" .. yaml)

-- 4. XML Serialization
-- "XML is for when you want your data to look like it was written in the 90s."
print("Serializing to XML...")
local xml = s.toXml(root)
print("XML Representation:\n" .. xml)

-- 5. Deserialization
-- "Reconstructing the tree. It's like building a LEGO set without the manual. 
-- But Quasar has a manual (this script)."
print("Testing Deserialization from JSON...")
local restored = s.fromJson(json)

if restored then
    print("Restored tree name: " .. restored:getName())
    local rConfig = restored:getChild("Config")
    if rConfig then
        local rID = rConfig:getChild("ID")
        if rID and rID:asLong() then
            print("Restored ID: " .. rID:asLong():value())
        end
    end
end

-- 6. Binary (BSON)
-- "Binary is for when you want your data to be fast and quiet. 
-- No humans allowed here. Only 1s and 0s."
print("Testing Binary Serialization...")
local bin = s.toBinary(root)
print("Binary data size: " .. #bin .. " bytes.")

local binRestored = s.fromBinary(bin)
if binRestored then
    print("Restored from binary: " .. binRestored:getName())
end

print("--- Serialization Finished ---")
-- "And that's how we travel through time. Or at least through files."
-- "Vogons would probably use XML. Don't be a Vogon."
