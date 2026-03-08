#include <gtest/gtest.h>
#include "quasar/scripting/LuaEngine.hpp"

using namespace quasar::scripting;

TEST(LuaEngineTest, BasicExecution) {
    LuaEngine engine;
    EXPECT_NO_THROW({
        engine.executeString("x = 10\ny = 20\nresult = x + y");
    });
    
    // Verify result in state
    int result = engine.getState()["result"];
    EXPECT_EQ(result, 30);
}

TEST(LuaEngineTest, FunctionDefinition) {
    LuaEngine engine;
    EXPECT_NO_THROW({
        engine.executeString(R"(
            function multiply(a, b)
                return a * b
            end
        )");
    });
    
    sol::protected_function multiply = engine.getState()["multiply"];
    EXPECT_TRUE(multiply.valid());
    
    int result = multiply(5, 6);
    EXPECT_EQ(result, 30);
}

TEST(LuaEngineTest, SandboxVerification) {
    LuaEngine engine;
    // In our LuaEngine, os functions should still be available until setupSandbox is called.
    
    // Call setupSandbox
    // Note: setupSandbox is currently protected, let's expose it via a test fixture or method 
    // Wait, setupSandbox is protected. For testing we might need a derived class.
}

class TestableLuaEngine : public LuaEngine {
public:
    void doSetupSandbox() {
        setupSandbox();
    }
};

TEST(LuaEngineTest, SandboxExecution) {
    TestableLuaEngine engine;
    engine.doSetupSandbox();
    
    // os.execute should be nil, so calling it in a script will result in a runtime error (attempt to call a nil value)
    sol::protected_function_result result = engine.executeString("os.execute('echo hi')");
    EXPECT_FALSE(result.valid());
}

TEST(LuaEngineTest, PanicHandling) {
    LuaEngine engine;
    // Attempting to execute malformed Lua should return an invalid result (syntax error)
    sol::protected_function_result result = engine.executeString("this is not valid lua code $#%#$");
    EXPECT_FALSE(result.valid());
}
