#include <gtest/gtest.h>
#include "quasar/scripting/ScriptExecutor.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <thread>
#include <chrono>

namespace quasar::scripting {

using namespace quasar::named;

TEST(ScriptExecutorTest, ExecuteOnce) {
    // Fire and forget
    EXPECT_NO_THROW({
        ScriptExecutor::ExecuteOnce("print('Standalone script executed')");
    });
}

TEST(ScriptExecutorTest, ExecuteSync) {
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    ScriptExecutor::ExecuteSync("x = 42", service);
    
    sol::protected_function_result result = service->execute("return x");
    int value = result.get<int>();
    EXPECT_EQ(value, 42);

    service->stop();
}

TEST(ScriptExecutorTest, ExecuteAsync) {
    std::shared_ptr<NamedObject> root = NamedObject::create("root");
    std::shared_ptr<LuaService> service = LuaService::create("TestService", root);
    service->start();

    std::string script = "y = 100";
    ScriptExecutor::ExecuteAsync(script, service);
    
    // Poll for result since async post might take a moment.
    int value = 0;
    for (int i = 0; i < 100; ++i) {
        sol::protected_function_result result = service->execute("return y or 0");
        value = result.get<int>();
        if (value == 100) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    EXPECT_EQ(value, 100);

    service->stop();
}

} // namespace quasar::scripting
