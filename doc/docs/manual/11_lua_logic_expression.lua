-- 11_lua_logic_expression.lua
-- The Brain: Boolean Algebra in Lua.
-- "Logic is the beginning of wisdom, not the end. Or so Spock says."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Lua Logic Expressions ---")

-- 1. Setup the inputs and outputs
local root = n.createObject("LogicBoard")
local inputA = n.createBoolean("InputA", false, root)
local inputB = n.createBoolean("InputB", true, root)
local output = n.createBoolean("Output", false, root)

-- 2. Define the logic as a NamedLuaMethod
-- "An AND gate is just a bouncer who checks both IDs."
local logicGate = n.createLuaMethod("EvaluateAND", function(owner, args)
    local a = owner:getChild("InputA"):asBoolean():value()
    local b = owner:getChild("InputB"):asBoolean():value()
    
    local result = a and b
    
    local outObj = owner:getChild("Output"):asBoolean()
    outObj:setValue(result)
    
    print("[Logic] Evaluated: " .. tostring(a) .. " AND " .. tostring(b) .. " = " .. tostring(result))
    return nil
end, root)

-- 3. Test the truth table
print("Testing Truth Table for AND Gate:")

inputA:setValue(false)
inputB:setValue(false)
logicGate:execute(nil)

inputA:setValue(false)
inputB:setValue(true)
logicGate:execute(nil)

inputA:setValue(true)
inputB:setValue(false)
logicGate:execute(nil)

inputA:setValue(true)
inputB:setValue(true)
logicGate:execute(nil)

print("Final output state: " .. tostring(output:value()))
print("--- Logic Expressions Finished ---")
