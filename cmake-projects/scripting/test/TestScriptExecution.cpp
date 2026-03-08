#include <gtest/gtest.h>
#include "quasar/scripting/ScriptExecutor.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include <thread>
#include <chrono>

namespace quasar::scripting {

class ScriptExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ScriptExecutorTest, ExecuteOnce) {
    // ExecuteOnce returns void, so we just check it runs without throwing
    EXPECT_NO_THROW(ScriptExecutor::ExecuteOnce("x = 42"));
}

TEST_F(ScriptExecutorTest, ExecuteSync) {
    LuaEngine engine;
    engine.executeString("x = 10");
    
    auto result = ScriptExecutor::ExecuteSync(engine, "return x * 2");
    ASSERT_TRUE(result.valid());
    EXPECT_EQ(result.get<int>(), 20);
}

TEST_F(ScriptExecutorTest, ExecuteAsync) {
    // Large computation or sleep to simulate work
    std::string script = "local sum = 0; for i=1,1000 do sum = sum + i end; return sum";
    
    LuaFuture future = ScriptExecutor::ExecuteAsync(script);
    
    // Do some "work" in C++ main thread
    int cpp_sum = 0;
    for (int i=1; i<=500; ++i) cpp_sum += i;
    
    future.wait();
    ASSERT_TRUE(future.isReady());
    
    auto result = future.get();
    ASSERT_TRUE(result.valid());
    EXPECT_EQ(result.get<int>(), 500500); // sum of 1 to 1000
    EXPECT_EQ(cpp_sum, 125250); // sum of 1 to 500
}

TEST_F(ScriptExecutorTest, ExecuteAsyncError) {
    LuaFuture future = ScriptExecutor::ExecuteAsync("error('async failure')");
    future.wait();
    
    auto result = future.get();
    ASSERT_FALSE(result.valid());
    
    // Check error message
    sol::error err = result;
    EXPECT_TRUE(std::string(err.what()).find("async failure") != std::string::npos);
}

TEST_F(ScriptExecutorTest, AsyncIsolation) {
    // Verify that async executions don't share state
    LuaFuture f1 = ScriptExecutor::ExecuteAsync("shared_var = 100; return shared_var");
    LuaFuture f2 = ScriptExecutor::ExecuteAsync("if shared_var == nil then return 'isolated' else return 'shared' end");
    
    f1.wait();
    f2.wait();
    
    EXPECT_EQ(f1.get().get<int>(), 100);
    EXPECT_EQ(f2.get().get<std::string>(), "isolated");
}

} // namespace quasar::scripting
