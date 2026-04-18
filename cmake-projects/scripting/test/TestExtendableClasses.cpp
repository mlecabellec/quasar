#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include <memory>

namespace quasar::scripting {

using namespace quasar::named;

/**
 * @brief Tests for C++ classes that can be extended with Lua logic.
 */
class ExtendableClassesTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = NamedObject::create("root");
    }

    std::shared_ptr<NamedObject> root;
};

TEST_F(ExtendableClassesTest, MethodOverride) {
    auto engine = LuaEngine::create();
    sol::state& lua = engine->getState();

    auto scriptable = ScriptableNamedObject::create("MyObject", root);
    lua["obj"] = LuaProxy<ScriptableNamedObject>(scriptable);

    // Override an event in Lua
    lua.script(R"(
        local mySelf = {
            count = 0,
            onEvent = function(self, event, data)
                self.count = self.count + 1
            end
        }
        obj:setLuaSelf(mySelf)
    )");

    scriptable->onEvent("test", sol::nil);
    scriptable->onEvent("test", sol::nil);

    // Check if Lua state was updated
    sol::table luaSelf = scriptable->getLuaSelf();
    int count = luaSelf["count"];
    EXPECT_EQ(count, 2);
    engine->shutdown();
}

TEST_F(ExtendableClassesTest, HookAddChild) {
    auto engine = LuaEngine::create();
    sol::state& lua = engine->getState();

    auto scriptable = ScriptableNamedObject::create("Parent", root);
    lua["obj"] = LuaProxy<ScriptableNamedObject>(scriptable);

    lua.script(R"(
        local mySelf = {
            onEvent = function(self, event, data)
                quasar.named.createObject("DynamicChild", obj)
            end
        }
        obj:setLuaSelf(mySelf)
    )");

    scriptable->onEvent("spawn", sol::nil);

    auto child = scriptable->getChild("DynamicChild");
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->getName(), "DynamicChild");
    engine->shutdown();
}

} // namespace quasar::scripting
