#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/TypeBindings.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
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

LuaEngine::LuaEngine() : LuaEngine(std::weak_ptr<LuaService>{}) {}

LuaEngine::LuaEngine(std::weak_ptr<LuaService> service) {
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
        sol::lib::io,
        sol::lib::os,
        sol::lib::utf8
    );

    // Register Quasar bindings
    bindCoreTypes(m_lua);
    bindNamedTypes(m_lua, service.lock());
}

LuaEngine::~LuaEngine() {
    // sol::state destroys the state automatically
}

sol::protected_function_result LuaEngine::executeString(const std::string& code) {
    return m_lua.safe_script(code, sol::script_pass_on_error);
}

void LuaEngine::setupSandbox() {
    // No-op as per user requirement: Lua should not be sandboxed and could use io functions.
}

void LuaEngine::gcStep(int step_size) {
    // Perform a garbage collection step
    // In Lua 5.4, lua_gc uses LUA_GCSTEP with a size
    lua_gc(m_lua.lua_state(), LUA_GCSTEP, step_size);
}

} // namespace scripting
} // namespace quasar
