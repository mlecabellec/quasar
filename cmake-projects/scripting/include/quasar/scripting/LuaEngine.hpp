#pragma once

// Include bundled Lua headers BEFORE sol2 to ensure we use the correct version
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <sol/sol.hpp>
#include <mutex>
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
class LuaEngine : public std::enable_shared_from_this<LuaEngine> {
public:
    /**
     * @brief Factory method to create a shared LuaEngine.
     * @return Shared pointer to the new engine.
     */
    static std::shared_ptr<LuaEngine> create();

    /**
     * @brief Factory method with service context.
     * @param service Weak pointer to the host service.
     * @return Shared pointer to the new engine.
     */
    static std::shared_ptr<LuaEngine> create(std::weak_ptr<LuaService> service);

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
     * @brief Gracefully shuts down the engine, clearing all Lua objects.
     */
    void shutdown();

    /**
     * @brief Performs a manual garbage collection step.
     * This is useful for real-time systems to avoid large GC pauses.
     * @param step_size The size of the GC step.
     */
    void gcStep(int step_size);

    /**
     * @brief Acquires a lock on the Lua state for thread-safe operations.
     * @return A unique_lock guarding the recursive mutex.
     */
    std::unique_lock<std::recursive_mutex> acquireLock() {
        return std::unique_lock<std::recursive_mutex>(m_mutex);
    }

    /**
     * @brief Exposes the raw recursive mutex for advanced yielding operations.
     */
    std::recursive_mutex& getMutex() { return m_mutex; }

    /**
     * @brief Gets the unique ID of this engine instance.
     */
    size_t getId() const { return m_id; }

protected:
    /**
     * @brief Constructs a new Lua Engine and initializes standard libraries.
     */
    LuaEngine();

    /**
     * @brief Constructs a new Lua Engine with a service context.
     * @param service Weak pointer to the host service.
     */
    explicit LuaEngine(std::weak_ptr<LuaService> service);

    /**
     * @brief Custom panic handler for the Lua state.
     */
    static int onPanic(lua_State* L);

    /**
     * @brief Sets up a safe sandbox environment.
     */
    void setupSandbox();

protected:
    sol::state m_lua;
    std::recursive_mutex m_mutex;
    size_t m_id;
};

} // namespace scripting
} // namespace quasar
