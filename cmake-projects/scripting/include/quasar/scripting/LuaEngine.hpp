#pragma once

// Include bundled Lua headers BEFORE sol2 to ensure we use the correct version
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <sol/sol.hpp>

#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace quasar {
namespace scripting {

/**
 * @brief Exception thrown when Lua execution fails.
 */
class LuaExecutionException : public std::runtime_error {
public:
    explicit LuaExecutionException(const std::string& message) : std::runtime_error(message) {}
};

class LuaService;

/**
 * @brief A wrapper around a sol::state, providing secure and controlled execution.
 */
class LuaEngine {
public:
    /**
     * @brief Constructs a new Lua Engine and initializes standard libraries.
     */
    LuaEngine();

    /**
     * @brief Constructs a new Lua Engine with a service context.
     * @param service Weak pointer to the host service.
     */
    LuaEngine(std::weak_ptr<LuaService> service);

    /**
     * @brief Destructor cleans up the Lua state.
     */
    virtual ~LuaEngine();

    /**
     * @brief Executes a string of Lua code safely.
     * @param code The Lua source code.
     * @return Result of the execution.
     */
    sol::protected_function_result executeString(const std::string& code);

    /**
     * @brief Access the underlying sol::state directly.
     * Use with caution.
     */
    sol::state& getState() { return m_lua; }

    /**
     * @brief Performs a manual garbage collection step.
     * This is useful for real-time systems to avoid large GC pauses.
     * @param step_size The size of the GC step.
     */
    void gcStep(int step_size);

protected:
    /**
     * @brief Custom panic handler for the Lua state.
     */
    static int onPanic(lua_State* L);

    /**
     * @brief Sets up a safe sandbox environment.
     */
    void setupSandbox();

private:
    sol::state m_lua;
};

} // namespace scripting
} // namespace quasar
