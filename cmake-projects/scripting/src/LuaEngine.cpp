#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/TypeBindings.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include <iostream>
#include <atomic>

namespace quasar {
namespace scripting {

/** @brief Atomic counter for unique engine identification. */
static std::atomic<size_t> s_engineIdCounter{0};

int LuaEngine::onPanic(lua_State* L) {
    const char* message = lua_tostring(L, -1);
    if (message) {
        std::cerr << "Lua PANIC: " << message << std::endl;
    } else {
        std::cerr << "Lua PANIC: Unknown error" << std::endl;
    }
    // Cannot easily throw a C++ exception through an unhandled Lua C panic without risking termination.
    // In sol2, panics only happen when unprotected API usage fails catastrophically or memory is exhausted.
    return 0;
}

LuaEngine::LuaEngine() : LuaEngine(std::weak_ptr<LuaService>{}) {}

LuaEngine::LuaEngine(std::weak_ptr<LuaService> service) : m_id(s_engineIdCounter.fetch_add(1)) {
    // [CS-0010.44] Set panic handler to capture fatal Lua errors.
    lua_atpanic(m_lua.lua_state(), &LuaEngine::onPanic);

    // Open libraries required for standard industrial scripting.
    m_lua.open_libraries(
        sol::lib::base, 
        sol::lib::package, 
        sol::lib::coroutine, 
        sol::lib::string, 
        sol::lib::math, 
        sol::lib::table, 
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::io,
        sol::lib::os,
        sol::lib::utf8
    );

    // Bind Quasar core and reflexive named object types.
    bindCoreTypes(m_lua);
    bindNamedTypes(m_lua, service.lock());

    // [CS-0010.30] Store raw engine pointer for internal C++ callbacks.
    m_lua["__quasar_engine"] = this;
}

std::shared_ptr<LuaEngine> LuaEngine::create() {
    // Private enabler to allow make_shared with protected constructor.
    struct make_shared_enabler : public LuaEngine {
        make_shared_enabler() : LuaEngine() {}
    };
    return std::make_shared<make_shared_enabler>();
}

std::shared_ptr<LuaEngine> LuaEngine::create(std::weak_ptr<LuaService> service) {
    // Private enabler for scoped creation.
    struct make_shared_enabler : public LuaEngine {
        explicit make_shared_enabler(std::weak_ptr<LuaService> s) : LuaEngine(s) {}
    };
    return std::make_shared<make_shared_enabler>(service);
}

LuaEngine::~LuaEngine() {
    std::unique_lock<std::recursive_mutex> lock = acquireLock();
    // [CS-0010.44] Invalidate all methods that might hold references to this state.
    ObjectTracker::getInstance().invalidateMethods(m_id);
}

sol::protected_function_result LuaEngine::executeString(const std::string& code) {
    std::unique_lock<std::recursive_mutex> lock = acquireLock();
    return m_lua.safe_script(code, sol::script_pass_on_error);
}

void LuaEngine::setupSandbox() {
    // No-op as per user requirement: Lua should not be sandboxed and could use io functions.
}

void LuaEngine::shutdown() {
    std::unique_lock<std::recursive_mutex> lock = acquireLock();
    // [CS-0010.44] Clear the global registry to force destruction of held Lua objects.
    // This must happen while the engine and bound C++ classes are still valid.
    m_lua["__quasar_engine"] = sol::nil;
    
    // [CS-0010.21] Invalidate all methods tied to THIS specific state.
    ObjectTracker::getInstance().invalidateMethods(m_id);

    // Release all strong references held by this specific engine.
    ObjectTracker::getInstance().untrackAll(m_id);

    // Trigger full garbage collection to run __gc metamethods.
    lua_gc(m_lua.lua_state(), LUA_GCCOLLECT, 0);
}

void LuaEngine::gcStep(int step_size) {
    // Perform a garbage collection step
    // In Lua 5.4, lua_gc uses LUA_GCSTEP with a size
    lua_gc(m_lua.lua_state(), LUA_GCSTEP, step_size);
}

} // namespace scripting
} // namespace quasar
