#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include <memory>

namespace quasar::scripting {

using namespace quasar::named;

class NamedLuaMethodTest : public ::testing::Test {
protected:
    /// @brief Set up the test suite.
    void SetUp() override {
        root = NamedObject::create("root");
        engine = LuaEngine::create();
    }
    
    /// @brief Clean up the test suite.
    void TearDown() override {
        engine->shutdown();
    }

    std::shared_ptr<NamedObject> root;
    std::shared_ptr<LuaEngine> engine;
};

TEST_F(NamedLuaMethodTest, Execution) {
    sol::state& lua = engine->getState();
    
    sol::function func = lua.load("return function(self, args) return args:asLong():value() * 2 end").call();
    
    std::shared_ptr<NamedLuaMethod> method = NamedLuaMethod::create("multiply", func, root);
    
    std::shared_ptr<NamedInteger<int64_t>> args = NamedInteger<int64_t>::create("args", 21);
    std::shared_ptr<NamedObject> result = method->execute(args);
    
    ASSERT_NE(result, nullptr);
    std::shared_ptr<NamedInteger<int64_t>> resInt = std::dynamic_pointer_cast<NamedInteger<int64_t>>(result);
    ASSERT_NE(resInt, nullptr);
    EXPECT_EQ(resInt->value(), 42);
}

TEST_F(NamedLuaMethodTest, Invalidation) {
    sol::state& lua = engine->getState();
    sol::function func = lua.load("return function(self, args) return 0 end").call();
    
    std::shared_ptr<NamedLuaMethod> method = NamedLuaMethod::create("test", func, root);
    
    // Simulate engine shutdown
    engine->shutdown();
    
    // Execution should now return nullptr gracefully
    std::shared_ptr<NamedObject> result = method->execute(nullptr);
    EXPECT_EQ(result, nullptr);
}

} // namespace quasar::scripting
