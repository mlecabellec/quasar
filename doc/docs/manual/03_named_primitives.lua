-- 03_named_primitives.lua
-- Storing Data: Primitive Types.
-- "In Quasar, numbers, strings, and booleans have names. They're more polite that way."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Named Primitives ---")

-- 1. Root container
local vault = n.createObject("DataVault")

-- 2. Long Integers
local age = n.createLong("UniverseAge", 42, vault)
print("The answer is: " .. age:value())
age:setValue(42000000000) -- Re-writing the age of the universe.
print("New age: " .. age:value())

-- 3. Floating Point (Double)
local speed = n.createDouble("StarshipSpeed", 0.0, vault)
speed:setValue(1.0) -- Warp factor 1, engaging.
print("Current speed: Warp " .. speed:value())

-- 4. Booleans (Newly bound!)
local isImprobable = n.createBoolean("ImprobabilityActive", true, vault)
print("Is the drive active? " .. tostring(isImprobable:value()))
isImprobable:setValue(false)
print("New state: " .. tostring(isImprobable:value()))

-- 5. Strings (Newly bound!)
local quote = n.createString("FamousQuote", "I'd far rather be happy than right.", vault)
print("Famous Quote: '" .. quote:value() .. "'")
quote:setValue("Don't Panic.")
print("Updated Quote: '" .. quote:value() .. "'")

-- 6. Temporals (Newly bound!)
local now = n.createTimestamp("LastUpdate", os.time(), vault)
print("Timestamp value (us): " .. tostring(now:value()))

local duration = n.createDuration("MissionLength", 3600, vault)
print("Duration value (us): " .. tostring(duration:value()))

local today = n.createDate("ArrivalDate", 12345, vault)
print("Date value (days since epoch): " .. tostring(today:value()))

-- 7. Casting
-- Any NamedObject can try to be a primitive.
local generic = vault:getChild("UniverseAge")
if generic then
    print("Found UniverseAge as generic object.")
    local asLong = generic:asLong()
    if asLong then
        print("Successfully cast to NamedLong! Value: " .. asLong:value())
    else
        print("FAILED to cast to NamedLong. This is not the answer you're looking for.")
    end
end

print("--- Primitives Finished ---")
-- "Life, the Universe, and Everything (in appropriate data types)."
