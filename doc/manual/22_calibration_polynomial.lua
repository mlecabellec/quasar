-- 22_calibration_polynomial.lua
-- The Curve Fitter: Polynomial Calibration.
-- "Sometimes the universe isn't a straight line. Often, it's a parabola."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: Polynomial Calibration Simulation ---")

local root = n.createObject("ComplexSensor")
local rawVal = n.createDouble("RawInput", 0.0, root)
local calVal = n.createDouble("CalibratedValue", 0.0, root)

-- 1. Polynomial Coefficients: c0 + c1*x + c2*x^2 + c3*x^3
local coeffs = {
    [0] = 0.5,    -- c0 (offset)
    [1] = 1.2,    -- c1 (linear)
    [2] = -0.05,  -- c2 (quadratic)
    [3] = 0.001   -- c3 (cubic)
}

-- 2. Define the Evaluation Method
local evaluatePolynomial = n.createLuaMethod("Calibrate", function(owner, args)
    local x = owner:getChild("RawInput"):asDouble():value()
    
    local result = 0.0
    local power = 1.0
    
    for i = 0, 3 do
        result = result + (coeffs[i] * power)
        power = power * x
    end
    
    owner:getChild("CalibratedValue"):asDouble():setValue(result)
    print(string.format("[PolyCal] Raw: %6.2f -> Calibrated: %8.4f", x, result))
    return nil
end, root)

-- 3. Test the polynomial curve
print("Testing Polynomial Calibration Curve:")
local inputs = { 0.0, 1.0, 5.0, 10.0, 20.0 }

for _, val in ipairs(inputs) do
    rawVal:setValue(val)
    evaluatePolynomial:execute(nil)
end

print("--- Polynomial Calibration Finished ---")
