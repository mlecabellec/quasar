#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/coretypes/IntegerTypes.hpp"
#include "quasar/coretypes/FloatingPointTypes.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedVariant.hpp"

using namespace quasar::scripting;
using namespace quasar::named;
using namespace quasar::coretypes;

class TypeBindingsTest : public ::testing::Test {
protected:
    LuaEngine engine;
};

TEST_F(TypeBindingsTest, IntegerPrecision) {
    // Large 64-bit integer test: 2^60 + 7
    int64_t largeVal = (1LL << 60) + 7;
    std::shared_ptr<Integer<int64_t>> valObj = std::make_shared<Integer<int64_t>>(largeVal);
    
    engine.getState()["val"] = valObj;
    
    // Check value in Lua
    engine.executeString("assert(val:value() == 1152921504606846983)");
    
    // Arithmetic in Lua - Use Long(10) instead of Long.new(10) to test call_constructor
    engine.executeString("res = val + Long(10)");
    
    Long res = engine.getState()["res"];
    EXPECT_EQ(res.value(), largeVal + 10);
    
    // Check toString
    engine.executeString("s = tostring(val)");
    std::string s = engine.getState()["s"];
    EXPECT_EQ(s, std::to_string(largeVal));
}

TEST_F(TypeBindingsTest, UnsignedIntegerPrecision) {
    // Max uint64_t test
    uint64_t maxVal = std::numeric_limits<uint64_t>::max();
    std::shared_ptr<Integer<uint64_t>> valObj = std::make_shared<Integer<uint64_t>>(maxVal);
    
    engine.getState()["uval"] = valObj;
    
    engine.executeString("s = uval:toString()");
    std::string s = engine.getState()["s"];
    EXPECT_EQ(s, std::to_string(maxVal));
}

TEST_F(TypeBindingsTest, QuantityMath) {
    engine.executeString(R"(
        m = Unit.fromSymbol("m")
        km = Unit.fromSymbol("km")
        
        q1 = Quantity(1.5, km) -- Test call_constructor
        q2 = Quantity.new(500.0, m) -- Test .new alias
        
        sum = q1 + q2
        val = sum:value()
        unit = sum:getUnit()
    )");
    
    double val = engine.getState()["val"];
    Unit unit = engine.getState()["unit"];
    
    // 1.5km + 500m = 2.0km 
    EXPECT_DOUBLE_EQ(val, 2.0);
    EXPECT_EQ(unit.symbol, "km");
}

TEST_F(TypeBindingsTest, HierarchyResolution) {
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::shared_ptr<NamedObject> c1 = NamedObject::create("c1", root);
    std::shared_ptr<NamedInteger<int64_t>> sub = NamedInteger<int64_t>::create("sub", 42, c1);
    
    engine.getState()["root"] = root;
    
    engine.executeString(R"(
        obj = quasar.resolve(root, "c1/sub")
        assert(obj ~= nil)
        assert(obj:getName() == "sub")
        assert(obj:getType() == "NamedInteger")
        
        -- Use safe casting helper
        nlong = obj:asLong()
        assert(nlong ~= nil)
        val = nlong:value()
    )");
    
    int64_t val = engine.getState()["val"];
    EXPECT_EQ(val, 42);
}

TEST_F(TypeBindingsTest, VariantHandling) {
    std::shared_ptr<NamedVariant> var = NamedVariant::create("var");
    engine.getState()["var"] = var;
    
    engine.executeString(R"(
        val = NamedLong.create("initial", 100)
        var:set(val)
        
        current = var:get()
        assert(current ~= nil)
        assert(current:getName() == "value")
        
        -- Use safe casting helper from NamedObject
        nlong = current:asLong()
        assert(nlong ~= nil)
        res = nlong:value()
    )");
    
    int64_t res = engine.getState()["res"];
    EXPECT_EQ(res, 100);
}

TEST_F(TypeBindingsTest, CppVariantHandling) {
    using NamedLong = NamedInteger<int64_t>;
    std::shared_ptr<NamedVariant> var = NamedVariant::create("var");
    std::shared_ptr<NamedLong> val = NamedLong::create("initial", 100);
    
    // This mimics what happens in Lua
    var->set(val);
    
    std::shared_ptr<NamedObject> current = var->get();
    ASSERT_NE(current, nullptr);
    EXPECT_EQ(current->getName(), "value");
    
    std::shared_ptr<NamedLong> nlong = std::dynamic_pointer_cast<NamedLong>(current);
    ASSERT_NE(nlong, nullptr);
    EXPECT_EQ(nlong->value(), 100);
}

TEST_F(TypeBindingsTest, IsolatedCreation) {
    engine.executeString("val = NamedLong.create('test', 123)");
    engine.executeString("assert(val:getName() == 'test')");
    engine.executeString("assert(val:asLong():value() == 123)");
}
