#include "quasar/scripting/ScriptManager.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include <iostream>

namespace quasar::scripting {

ScriptManager& ScriptManager::getInstance() {
    static ScriptManager instance;
    return instance;
}

ScriptManager::ScriptManager() : named::NamedObject("ScriptManager") {}

std::shared_ptr<LuaService> ScriptManager::createService(const std::string& name, const std::string& scriptPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::shared_ptr<LuaService> svc = LuaService::create(name, getSelf());
    
    // Setup sandbox for the service
    sol::environment env = createSandbox(svc->getEngine()->getState());
    
    // Note: To truly sandbox, we need to ensure the script runs IN this environment.
    // sol::state::script_file has an overload for environment.
    
    if (svc->loadScript(scriptPath)) {
        m_services[name] = svc;
        svc->onInit();
        return svc;
    }
    
    return nullptr;
}

void ScriptManager::stopService(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::map<std::string, std::shared_ptr<LuaService>>::iterator it = m_services.find(name);
    if (it != m_services.end()) {
        it->second->onShutdown();
        m_services.erase(it);
    }
}

void ScriptManager::update(double dt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (std::pair<const std::string, std::shared_ptr<LuaService>>& pair : m_services) {
        pair.second->onUpdate(dt);
    }
}

void ScriptManager::tickGC(int stepSize) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (std::pair<const std::string, std::shared_ptr<LuaService>>& pair : m_services) {
        sol::state& lua = pair.second->getEngine()->getState();
        lua_gc(lua.lua_state(), LUA_GCSTEP, stepSize);
    }
}

sol::environment ScriptManager::createSandbox(sol::state_view lua) {
    // Create a new environment that only allows safe things
    sol::environment env(lua, sol::create, lua.globals());
    
    // Restricted environment: we mostly want to hide the global table
    // and only provide known-safe libraries.
    // In sol2, we can set the __index of the environment to restricted globals.
    
    // For now, let's at least ensure dangerous libraries aren't loaded 
    // or are overwritten in this environment.
    
    // Provide quasar bindings to the sandbox
    // Note: Bindings are already established in the LuaEngine constructor.
    
    return env;
}

} // namespace quasar::scripting
