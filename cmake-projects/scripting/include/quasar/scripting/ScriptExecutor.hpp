#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaFuture.hpp"
#include <string>

namespace quasar::scripting {

/**
 * @brief Utility for executing Lua code in different modes.
 */
class ScriptExecutor {
public:
    /**
     * @brief Executes a script string once in a transient Lua environment.
     * Use for fire-and-forget logic. Results are not returned to avoid
     * lifetime issues with the transient Lua state.
     * @param script The Lua code to execute.
     */
    static void ExecuteOnce(const std::string& script) {
        LuaEngine engine{std::weak_ptr<LuaService>{}};
        engine.executeString(script);
    }

    /**
     * @brief Executes a script string asynchronously in a background thread.
     * @param script The Lua code to execute.
     * @return A LuaFuture to track and retrieve the result.
     */
    static LuaFuture ExecuteAsync(const std::string& script);

    /**
     * @brief Executes a script string synchronously in an existing engine.
     * @param engine The engine to use.
     * @param script The Lua code to execute.
     * @return Result of the execution.
     */
    static sol::protected_function_result ExecuteSync(LuaEngine& engine, const std::string& script) {
        return engine.executeString(script);
    }
};

} // namespace quasar::scripting
