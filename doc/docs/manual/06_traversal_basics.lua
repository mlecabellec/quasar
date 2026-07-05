-- 06_traversal_basics.lua
-- The Art of Navigation: Paths and Resolvers.
-- "If you don't know where you're going, any path will take you there.
-- But in Quasar, you usually want a specific sensor value."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Traversal and Resolving ---")

-- 1. Setup a complex tree
local ship = n.createObject("Nostromo")
local deckA = n.createObject("DeckA", ship)
local deckB = n.createObject("DeckB", ship)

local engine = n.createObject("Engine", deckB)
local temp = n.createDouble("Temperature", 25.5, engine)
local status = n.createString("Status", "Normal", engine)

-- 2. Basic Traversal
print("DeckB has " .. #deckB:getChildren() .. " children.")
local e = deckB:getChild("Engine")
if e then
    print("Found Engine under DeckB.")
end

-- 3. Using quasar.resolve (Absolute-like and relative paths)
-- "Resolving paths is like following a recipe. If you miss a 'Deck', you get no cake."
print("Testing quasar.resolve...")

-- Relative to ship
local node = quasar.resolve(ship, "DeckB/Engine/Temperature")
if node then
    print("Resolved Temperature: " .. node:asDouble():value())
else
    print("FAILED to resolve Temperature path.")
end

-- 4. Path with leading/trailing slashes handling
local node2 = quasar.resolve(ship, "/DeckB//Engine///Status/")
if node2 then
    print("Resolved Status: " .. node2:asString():value())
end

-- 5. Parent Traversal
local p1 = temp:getParent()
local p2 = p1:getParent()
local p3 = p2:getParent()
print("Temperature's great-grandparent is: " .. p3:getName())

-- 6. Searching for non-existent paths
local ghost = quasar.resolve(ship, "DeckA/Alien")
if not ghost then
    print("No Alien found on DeckA. Safe... for now.")
end

print("--- Traversal Finished ---")
-- "In space, no one can hear you resolve a null pointer. 
-- Luckily, Quasar returns nil instead of crashing."
