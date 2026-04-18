#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaFuture.hpp"
#include <string>
#include <memory>

namespace quasar::scripting {

class LuaService;

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
    static void ExecuteOnce(const std::string& script);

    /**
     * @brief Executes a script string asynchronously in a service thread.
     * @param script The Lua code to execute.
     * @param service The host service.
     */
    static void ExecuteAsync(const std::string& script, std::shared_ptr<LuaService> service);

    /**
     * @brief Executes a script string synchronously in a service thread.
     * @param script The Lua code to execute.
     * @param service The host service.
     */
    static void ExecuteSync(const std::string& script, std::shared_ptr<LuaService> service);
};

} // namespace quasar::scripting
