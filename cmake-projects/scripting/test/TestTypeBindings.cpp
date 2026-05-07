#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedVariant.hpp"
#include <memory>

namespace quasar::scripting {

using namespace quasar::named;

class TypeBindingsTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = NamedObject::create("root");
        engine = LuaEngine::create();
    }
    
    void TearDown() override {
        engine->shutdown();
    }

    std::shared_ptr<NamedObject> root;
    std::shared_ptr<LuaEngine> engine;
};

TEST_F(TypeBindingsTest, IntegerPrecision) {
    sol::state& lua = engine->getState();
    auto val = NamedInteger<int64_t>::create("val", 123456789012345LL, root);
    lua["val"] = LuaProxy<NamedInteger<int64_t>>(val);

    sol::protected_function_result result = lua.script("return val:value()");
    ASSERT_TRUE(result.valid());
    int64_t retrieved = result;
    EXPECT_EQ(retrieved, 123456789012345LL);
}

TEST_F(TypeBindingsTest, UnsignedIntegerPrecision) {
    sol::state& lua = engine->getState();
    auto val = NamedInteger<uint64_t>::create("val", 18446744073709551610ULL, root);
    lua["val"] = LuaProxy<NamedInteger<uint64_t>>(val);

    sol::protected_function_result result = lua.script("return val:value()");
    ASSERT_TRUE(result.valid());
    uint64_t retrieved = result;
    EXPECT_EQ(retrieved, 18446744073709551610ULL);
}

TEST_F(TypeBindingsTest, QuantityMath) {
    sol::state& lua = engine->getState();
    auto q = NamedQuantity::create("speed", 100.0, "km/h", root);
    lua["q"] = LuaProxy<NamedQuantity>(q);

    lua.script("newVal = q:value() * 2");
    double newVal = lua["newVal"];
    EXPECT_DOUBLE_EQ(newVal, 200.0);
}

TEST_F(TypeBindingsTest, HierarchyResolution) {
    sol::state& lua = engine->getState();
    std::shared_ptr<NamedObject> child = NamedObject::create("child", root);
    std::shared_ptr<NamedObject> grandchild = NamedObject::create("grandchild", child);
    
    lua["root"] = LuaProxy<NamedObject>(root);
    
    sol::protected_function_result result = lua.script("return quasar.resolve(root, 'child/grandchild')");
    ASSERT_TRUE(result.valid());
    
    sol::object obj = result;
    auto resolved = extractNamedObject(obj);
    ASSERT_NE(resolved, nullptr); 
    EXPECT_EQ(resolved->getName(), "grandchild");
}

TEST_F(TypeBindingsTest, VariantHandling) {
    sol::state& lua = engine->getState();
    auto v = NamedVariant::create("v", coretypes::Variant(), root);
    lua["v"] = LuaProxy<NamedVariant>(v);

    lua.script(R"(
        local obj = quasar.named.createObject("Content")
        v:set(obj)
    )");

    auto content = v->get();
    ASSERT_NE(content, nullptr);
    EXPECT_EQ(content->getName(), "value");
}

TEST_F(TypeBindingsTest, CppVariantHandling) {
    sol::state& lua = engine->getState();
    auto v = NamedVariant::create("v", coretypes::Variant(), root);
    auto content = NamedObject::create("CppContent");
    v->set(content);
    
    lua["v"] = LuaProxy<NamedVariant>(v);

    sol::protected_function_result result = lua.script("return v:get():getName()");
    ASSERT_TRUE(result.valid());
    std::string name = result;
    EXPECT_EQ(name, "value");
}

TEST_F(TypeBindingsTest, IsolatedCreation) {
    sol::state& lua = engine->getState();
    
    lua.script(R"(
        local obj = quasar.named.createObject("Orphan")
        name = obj:getName()
    )");
    
    std::string name = lua["name"];
    EXPECT_EQ(name, "Orphan");
}

} // namespace quasar::scripting
