-- 01_hello_world.lua
-- The most basic script to verify the environment.
-- "In the beginning, there was a print statement."

print("--- Quasar Script Runner: Hello World ---")
print("Greeting from the Sirius Cybernetics Corporation (Lua Division).")
print("Your environment seems to be working correctly.")
print("Current Lua version: " .. _VERSION)

-- In Quasar, we often use the 'quasar' global table.
if quasar then
    print("Quasar module found!")
else
    print("Warning: Quasar module NOT found. Are you running this via 'sre'?")
end

print("--- End of Hello World ---")
-- "So long, and thanks for all the fish!"
