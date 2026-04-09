-- 09_named_service_params.lua
-- Special Delivery: Passing Arguments and States.
-- "If you want someone to do something, you have to tell them how much of it to do.
-- Usually in a NamedObject."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Methods and Arguments ---")

-- 1. Create a root
local root = n.createObject("Restaurant")

-- 2. Define a method that takes a Map of arguments
-- "The Chef only works if you give him a clear order. No vague requests here."
local chef = n.createLuaMethod("Cook", function(owner, args)
    if not args then
        print("[Chef] I can't cook nothing! I need an order.")
        return nil
    end
    
    local dish = "Something mysterious"
    local qty = 1
    
    -- Check if args is a Map
    if args:getType() == "NamedMap" then
        local m = args:asMap()
        local dObj = m:get("Dish")
        if dObj and dObj:asString() then
            dish = dObj:asString():value()
        end
        local qObj = m:get("Quantity")
        if qObj and qObj:asLong() then
            qty = qObj:asLong():value()
        end
    end
    
    print("[Chef] Cooking " .. qty .. " " .. dish .. "(s). Hot and fresh!")
    
    -- Return a response
    local response = n.createString("OrderResponse", "Enjoy your " .. dish .. "!")
    return response
end, root)

-- 3. Creating the argument object (A NamedMap)
print("Placing an order...")
local order = n.createMap("Order")
n.createString("Dish", "Pan Galactic Gargle Blaster", order)
n.createLong("Quantity", 2, order)

-- 4. Execute with arguments
local result = chef:execute(order)

if result and result:asString() then
    print("Waiter: '" .. result:asString():value() .. "'")
end

-- 5. Using a Service with internal params
-- "A service can also have parameters that it watches in the tree."
local oven = n.createService("TheOven", root)
local temp = n.createLong("TargetTemp", 200, oven)
local current = n.createLong("CurrentTemp", 20, oven)

n.createLuaMethod("run", function(owner, args)
    local target = owner:getChild("TargetTemp"):asLong():value()
    local currObj = owner:getChild("CurrentTemp"):asLong()
    local curr = currObj:value()
    
    if curr < target then
        currObj:setValue(curr + 10)
        print("[Oven] Heating up... " .. currObj:value() .. "°C")
    else
        print("[Oven] Target temperature reached. Baking complete!")
    end
end, oven)

oven:setCycleTime(200)
oven:start()

os.execute("sleep 1.0")
print("Chef: 'Turn up the heat to 250!'")
temp:setValue(250)
os.execute("sleep 1.0")

oven:stop()

print("--- Parameters Finished ---")
-- "Cooking with Quasar. A recipe for success."
