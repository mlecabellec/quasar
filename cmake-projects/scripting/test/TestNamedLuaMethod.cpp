#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"

using namespace quasar::scripting;
using namespace quasar::named;

class NamedLuaMethodTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_shared<LuaEngine>();
        root = NamedObject::create("root");
    }

    std::shared_ptr<LuaEngine> engine;
    std::shared_ptr<NamedObject> root;
};

TEST_F(NamedLuaMethodTest, LuaProxyBasic) {
    sol::state& lua = engine->getState();
    auto obj = NamedObject::create("simple");
    lua["myObj"] = LuaProxy<NamedObject>(obj);
    
    // Check if getName() works
    auto result = lua.safe_script("return myObj:getName()");
    ASSERT_TRUE(result.valid());
    std::string name = result;
    EXPECT_EQ(name, "simple");
}

TEST_F(NamedLuaMethodTest, LuaMethodExecution) {
    sol::state& lua = engine->getState();
    
    // Define a Lua function that adds two numbers from args
    lua.script(R"(
        function add(owner, args)
            local a = args:asLong():value()
            local b = owner:getChild("val"):asLong():value()
            return quasar.named.createLong("result", a + b)
        end
    )");

    auto val = NamedInteger<int64_t>::create("val", 10, root);
    sol::function func = lua["add"];
    auto method = NamedLuaMethod::create("addMethod", func, root);

    auto args = NamedInteger<int64_t>::create("args", 32);
    auto result = method->execute(args);
    auto resultInt = std::dynamic_pointer_cast<NamedInteger<int64_t>>(result);

    ASSERT_NE(resultInt, nullptr);
    EXPECT_EQ(resultInt->value(), 42);
}

TEST_F(NamedLuaMethodTest, LuaMethodNoReturn) {
    sol::state& lua = engine->getState();
    lua.script(R"(
        function touch(owner, args)
            owner:getChild("flag"):asLong():setName("touched")
        end
    )");

    auto flag = NamedInteger<int64_t>::create("flag", 0, root);
    sol::function func = lua["touch"];
    auto method = NamedLuaMethod::create("touchMethod", func, root);

    method->execute(nullptr);
    EXPECT_EQ(flag->getName(), "touched");
}

TEST_F(NamedLuaMethodTest, LuaServiceIntegration) {
    sol::state& lua = engine->getState();
    sol::protected_function_result pfr = engine->executeString(R"(
        local service = quasar.named.createService("luaService", nil)
        local counter = quasar.named.createLong("counter", 0, service)
        
        quasar.named.createLuaMethod("run", function(owner, args)
            local c = owner:getChild("counter"):asLong()
            c:setValue(c:value() + 1)
        end, service)
        
        service:setCycleTime(10)
        return service
    )");
    ASSERT_TRUE(pfr.valid());
    sol::object res = pfr;
    lua["service_obj"] = res;
    
    // Call start from C++ instead of Lua to see if it makes a difference.
    engine->executeString("service_obj:start()");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    engine->executeString("service_obj:stop()");
    
    auto serviceProxy = res.as<LuaProxy<NamedObject>>();
    auto service = std::dynamic_pointer_cast<NamedService>(serviceProxy.lock());
    ASSERT_NE(service, nullptr);
    
    auto counter = std::dynamic_pointer_cast<NamedInteger<int64_t>>(service->getChild("counter"));
    ASSERT_NE(counter, nullptr);
    EXPECT_GE(counter->value(), 3);
}
