#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"

namespace quasar::scripting {

class ExtendableClassesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ExtendableClassesTest, MethodOverride) {
    LuaEngine engine;
    sol::state& lua = engine.getState();
    
    // Create a scriptable object
    auto obj = ScriptableNamedObject::create("MyObject");
    
    // Define override in Lua
    lua["obj"] = obj;
    lua.script(R"(
        local self_table = {
            getType = function(self) return "LuaCustomType" end,
            onEvent = function(self, name, data) self.last_event = name end
        }
        obj:setLuaSelf(self_table)
    )");
    
    // Test C++ call -> Lua override
    // getType() is virtual in NamedObject, overridden in ScriptableNamedObject to call Lua
    EXPECT_EQ(obj->getType(), "LuaCustomType");
    
    // Test event hook
    obj->onEvent("Start", sol::nil);
    lua.script("last_event = obj:getLuaSelf().last_event");
    EXPECT_EQ(lua["last_event"].get<std::string>(), "Start");
}

TEST_F(ExtendableClassesTest, HookAddChild) {
    LuaEngine engine;
    sol::state& lua = engine.getState();
    
    auto parent = ScriptableNamedObject::create("Parent");
    lua["parent"] = parent;
    lua.script(R"(
        parent:setLuaSelf({
            onAddChild = function(self, child) 
                self.child_name = child:getName() 
            end
        })
    )");
    
    auto child = named::NamedObject::create("Child");
    // setParent calls parent->addChild internally
    child->setParent(parent); 
    
    lua.script("child_name = parent:getLuaSelf().child_name");
    EXPECT_EQ(lua["child_name"].get<std::string>(), "Child");
}

} // namespace quasar::scripting
