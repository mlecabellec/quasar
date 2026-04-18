#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaProxy.hpp"

using namespace quasar::scripting;

TEST(LuaEngineTest, BasicExecution) {
    auto engine = LuaEngine::create();
    EXPECT_NO_THROW({
        engine->executeString("x = 10\ny = 20\nresult = x + y");
    });
    
    // Verify result in state
    int result = engine->getState()["result"];
    EXPECT_EQ(result, 30);
    engine->shutdown();
}

TEST(LuaEngineTest, FunctionDefinition) {
    auto engine = LuaEngine::create();
    EXPECT_NO_THROW({
        engine->executeString(R"(
            function multiply(a, b)
                return a * b
            end
        )");
    });
    
    sol::protected_function multiply = engine->getState()["multiply"];
    EXPECT_TRUE(multiply.valid());
    
    int result = multiply(5, 6);
    EXPECT_EQ(result, 30);
    engine->shutdown();
}

class TestableLuaEngine : public LuaEngine {
public:
    static std::shared_ptr<TestableLuaEngine> create() {
        struct enabler : public TestableLuaEngine {};
        return std::make_shared<enabler>();
    }
    void doSetupSandbox() {
        setupSandbox();
    }
};

TEST(LuaEngineTest, SandboxExecution) {
    auto engine = TestableLuaEngine::create();
    engine->doSetupSandbox();
    
    // As per user requirement, sandboxing is disabled, so os.execute should STILL be valid.
    sol::protected_function_result result = engine->executeString("os.execute('echo hi > /dev/null')");
    EXPECT_TRUE(result.valid());
    engine->shutdown();
}

TEST(LuaEngineTest, PanicHandling) {
    auto engine = LuaEngine::create();
    // Attempting to execute malformed Lua should return an invalid result (syntax error)
    sol::protected_function_result result = engine->executeString("this is not valid lua code $#%#$");
    EXPECT_FALSE(result.valid());
    engine->shutdown();
}
