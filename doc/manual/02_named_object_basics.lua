-- 02_named_object_basics.lua
-- Exploring the Tree: The Hitchhiker's Guide to NamedObject.
-- "Everything in Quasar is a NamedObject, or it's just plain boring."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: NamedObject Basics ---")

-- 1. Creating the Root
-- Roots are special because they have no parent.
-- "In the beginning, the Root was created. This has made a lot of people very angry
-- and been widely regarded as a bad move."
local root = n.createObject("HeartOfGold")
print("Root object created: " .. root:getName())

-- 2. Creating Children
-- You can attach objects to others to build a tree.
local bridge = n.createObject("Bridge", root)
local engineRoom = n.createObject("InfiniteImprobabilityDrive", root)

print("Child count of Root: " .. #root:getChildren())

-- 3. Navigation
-- You can find objects by name.
local foundBridge = root:getChild("Bridge")
if foundBridge then
    print("Found the " .. foundBridge:getName() .. " via the Root.")
end

-- 4. Hierarchy Depth
local terminal = n.createObject("ControlPanel", bridge)
print("Terminal full path (logical): /" .. root:getName() .. "/" .. bridge:getName() .. "/" .. terminal:getName())

-- 5. Introspection
print("Terminal type: " .. terminal:getType())
if terminal:getParent() then
    print("Terminal parent: " .. terminal:getParent():getName())
end

-- 6. Lists
print("Listing all children of Root:")
for i, child in ipairs(root:getChildren()) do
    print(" [" .. i .. "] Name: " .. child:getName() .. " (Type: " .. child:getType() .. ")")
end

print("--- Basics Finished ---")
-- "Don't Panic, and always carry a towel (or a Root object)."
