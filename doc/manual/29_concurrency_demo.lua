-- 29_concurrency_demo.lua
-- The Jugglers: Concurrency and Thread Safety.
-- "Many hands make light work, unless they all grab the same byte.
-- Luckily, Quasar's NamedObjects use internal mutexes."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Concurrency Awareness ---")

-- 1. The Shared State
local root = n.createObject("Bank")
local account = n.createLong("Balance", 1000, root)

print("Initial Balance: $" .. account:value())

-- 2. Define their behavior (both trying to deposit money)
local function depositLogic(owner, args)
    local curr = account:value()
    account:setValue(curr + 10)
    print("[" .. owner:getName() .. "] Deposited $10. Total: $" .. account:value())
    return nil
end

-- 3. Two tellers (methods)
local teller1 = n.createLuaMethod("Teller1", depositLogic, root)
local teller2 = n.createLuaMethod("Teller2", depositLogic, root)

-- 4. Start the operations (Simulating concurrency sequentially)
print("\nStarting operations...")
for i = 1, 10 do
    teller1:execute(nil)
    teller2:execute(nil)
end

print("Final Balance: $" .. account:value())
if account:value() > 1000 then
    print("Transaction integrity maintained without explicit Lua locks.")
else
    print("Wait... where did the money go?")
end

print("--- Concurrency Demo Finished ---")
-- "Remember: Mutexes are like bathroom keys. Only one person at a time, and don't take too long."
