-- 04_named_containers.lua
-- Organized Chaos: Arrays and Maps.
-- "Organizing things is the first step towards losing them. Or find them faster."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Named Containers ---")

local root = n.createObject("Warehouse")

-- 1. NamedArray
-- "An ordered collection of things. Like the queue at the post office, but faster."
local items = n.createArray("Boxes", root)

-- Add some content (NamedObjects) to the array by making them children.
-- NOTE: In this version of Quasar, children are added by name. 
-- In an array, we might just create objects with the array as parent.
-- Let's check if we can add children to the array.
local box1 = n.createObject("Box0", items)
local box2 = n.createObject("Box1", items)

print("Array size: " .. items:size())

-- Getting elements
local retrieved = items:get(1) -- Lua is 1-indexed for the 'get' proxy method.
if retrieved then
    print("Box at index 1: " .. retrieved:getName())
end

-- 2. NamedMap
-- "Mapping keys to values. Like a phonebook, but without the annoying ads."
local dictionary = n.createMap("PhoneBook", root)

local entry1 = n.createObject("Marvin", dictionary)
local entry2 = n.createObject("Zaphod", dictionary)

print("Map size: " .. dictionary:size())

-- Getting from map
local retrievedMarvin = dictionary:get("Marvin")
if retrievedMarvin then
    print("Retrieved from map: " .. retrievedMarvin:getName())
end

-- 3. Nesting Containers
local nested = n.createArray("NestedArray", items) -- Array of Arrays? Why not.
local subBox = n.createObject("SubBox", nested)
print("Nested box found? " .. tostring(nested:get(1) ~= nil))

print("--- Containers Finished ---")
-- "Everything in its right place. Or at least, with a name and a parent."
