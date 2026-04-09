-- 30_the_ultimate_answer.lua
-- The Grand Finale: Bringing It All Together.
-- "To find the ultimate answer, you just need a big enough computer...
-- or a very clever Quasar script."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: The Ultimate Answer ---")

local root = n.createObject("DeepThought")

-- 1. Complex State
local question = n.createString("TheQuestion", "Unknown", root)
local answer = n.createLong("TheAnswer", 0, root)
local isComputing = n.createBoolean("Computing", true, root)

-- 2. Logic & Calibration mixed
local calibrateMethod = n.createLuaMethod("CalibrateTruth", function(owner, args)
    local val = owner:getChild("TheAnswer"):asLong():value()
    -- Apply the "Hitchhiker" polynomial:
    -- Truth = (Val * 0) + 42
    return n.createLong("Truth", (val * 0) + 42)
end, root)

-- 3. Execution Loop
print("[DeepThought] I am thinking. Come back in 7 and a half million years.")
isComputing:setValue(true)

while isComputing:value() do
    local current = answer:value()
    
    if current < 42 then
        answer:setValue(current + 1)
        print("[DeepThought] Calculating... " .. answer:value())
        if quasar.sleep then quasar.sleep(20) end
    else
        isComputing:setValue(false)
        -- We found it! Calibrate the truth.
        local truth = root:getChild("CalibrateTruth"):asMethod():execute(nil)
        print("\n[DeepThought] The Answer to Life, the Universe, and Everything is: " .. truth:asLong():value())
    end
end

print("\n--- The Ultimate Answer Finished ---")
-- "Now, we just need to figure out what the actual question was."
