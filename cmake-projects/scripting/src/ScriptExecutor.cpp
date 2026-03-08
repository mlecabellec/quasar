#include "quasar/scripting/ScriptExecutor.hpp"
#include <sol/sol.hpp>
#define ASIO_STANDALONE
#include <asio/thread_pool.hpp>
#include <asio/post.hpp>
#include <thread>

namespace quasar::scripting {

// Use a static thread pool for background Lua execution.
static asio::thread_pool g_luaThreadPool(std::thread::hardware_concurrency());

LuaFuture ScriptExecutor::ExecuteAsync(const std::string& script) {
    auto engine = std::make_shared<LuaEngine>();
    auto promise = std::make_shared<std::promise<sol::protected_function_result>>();
    auto future = promise->get_future();

    asio::post(g_luaThreadPool, [engine, script, promise]() {
        try {
            // Execution on background thread.
            sol::protected_function_result result = engine->executeString(script);
            // Must std::move because protected_function_result is move-only.
            promise->set_value(std::move(result));
        } catch (const std::exception& e) {
            promise->set_exception(std::make_exception_ptr(e));
        } catch (...) {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("Unknown error in Lua background execution")));
        }
    });

    return LuaFuture(engine, std::move(future));
}

} // namespace quasar::scripting
