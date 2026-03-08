#include "quasar/scripting/LuaEngine.hpp"
#include <iostream>

namespace quasar {
namespace scripting {

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

LuaEngine::LuaEngine() {
    // Set panic handler before doing anything else
    lua_atpanic(m_lua.lua_state(), &LuaEngine::onPanic);

    // Open libraries we consider safe by default.
    // Notice we DO NOT open os or io by default for sandboxing purposes,
    // though for now we open standard basic libraries.
    m_lua.open_libraries(
        sol::lib::base, 
        sol::lib::package, 
        sol::lib::coroutine, 
        sol::lib::string, 
        sol::lib::math, 
        sol::lib::table, 
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::io, // TODO: Restrict later for strict sandboxing
        sol::lib::os  // TODO: Restrict later for strict sandboxing
    );

    // Provide a global error handler for protected function calls
    m_lua.set_function("quasar_error_handler", [](std::string message) {
        return "Lua Error: " + message;
    });

}

LuaEngine::~LuaEngine() {
    // sol::state destroys the state automatically
}

void LuaEngine::setupSandbox() {
    // Basic sandboxing example, removing dangerous os methods
    m_lua["os"]["execute"] = sol::lua_nil;
    m_lua["os"]["remove"] = sol::lua_nil;
    m_lua["os"]["rename"] = sol::lua_nil;
    m_lua["os"]["exit"] = sol::lua_nil;
    
    // Similarly for io if needed, eg io.popen
    m_lua["io"]["popen"] = sol::lua_nil;
}

void LuaEngine::executeString(const std::string& code) {
    auto result = m_lua.safe_script(code, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        throw LuaExecutionException(err.what());
    }
}

void LuaEngine::gcStep(int step_size) {
    // Perform a garbage collection step
    // In Lua 5.4, lua_gc uses LUA_GCSTEP with a size
    lua_gc(m_lua.lua_state(), LUA_GCSTEP, step_size);
}

} // namespace scripting
} // namespace quasar
