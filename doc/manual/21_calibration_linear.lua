-- 21_calibration_linear.lua
-- The Adjuster: Linear Calibration in Lua.
-- "Sensors lie. Calibration is the art of telling them exactly how much they lie."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Linear Calibration Simulation ---")

local root = n.createObject("SensorNode")
local rawAdc = n.createLong("RawADC", 0, root)
local calibrated = n.createDouble("CalibratedValue", 0.0, root)

-- 1. Linear Calibration Formula: y = mx + b
local m = 0.015 -- Scale (e.g., 0.015 V per tick)
local b = -0.5  -- Offset

-- 2. Define the Calibration Method
local calibrate = n.createLuaMethod("ApplyLinearCalibration", function(owner, args)
    local raw = owner:getChild("RawADC"):asLong():value()
    
    -- Apply y = mx + b
    local result = (raw * m) + b
    
    owner:getChild("CalibratedValue"):asDouble():setValue(result)
    print(string.format("[Calibration] Raw ADC: %4d -> Calibrated: %7.3f", raw, result))
    return nil
end, root)

-- 3. Run the calibration on some mock data
print("Applying linear calibration (m=" .. m .. ", b=" .. b .. "):")

local testValues = { 0, 100, 500, 1024, 2048, 4095 }

for _, val in ipairs(testValues) do
    rawAdc:setValue(val)
    calibrate:execute(nil)
end

print("--- Linear Calibration Finished ---")
