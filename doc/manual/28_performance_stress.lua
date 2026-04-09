-- 28_performance_stress.lua
-- The Marathon: Performance Stress Test.
-- "How many NamedObjects can we create before the computer complains?"

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Performance Stress Test ---")

local root = n.createObject("StressRoot")

-- 1. Create 10,000 objects in a hierarchy
local numObjects = 10000
print(string.format("Attempting to create %d NamedLongs...", numObjects))

local start_time = os.time()

-- We'll attach them all to root to see how it handles a flat list
for i = 1, numObjects do
    local objName = string.format("Item_%05d", i)
    n.createLong(objName, i, root)
end

local end_time = os.time()
local elapsed = os.difftime(end_time, start_time)

print(string.format("Created %d objects in %d seconds.", numObjects, elapsed))

-- 2. Traversal benchmark
print("\nAttempting to resolve 1,000 objects dynamically...")
local res_start = os.time()
local found = 0

for i = 1, 1000 do
    local targetName = string.format("Item_%05d", math.random(1, numObjects))
    local node = root:getChild(targetName)
    if node then
        found = found + 1
    end
end

local res_end = os.time()
local res_elapsed = os.difftime(res_end, res_start)

print(string.format("Resolved %d/%d objects in %d seconds.", found, 1000, res_elapsed))

print("\n--- Performance Stress Finished ---")
-- "It's not about how fast you run, it's about how fast your tree allocates memory."
