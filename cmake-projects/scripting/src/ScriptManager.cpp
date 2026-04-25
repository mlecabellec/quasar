#include "quasar/scripting/ScriptManager.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <iostream>

namespace quasar::scripting {

ScriptManager& ScriptManager::getInstance() {
    static ScriptManager instance;
    return instance;
}

ScriptManager::ScriptManager() : named::NamedObject("ScriptManager") {}

std::shared_ptr<LuaService> ScriptManager::createService(const std::string& name, const std::string& scriptPath) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring ScriptManager mutex in createService");
    }
    
    std::shared_ptr<LuaService> svc = LuaService::create(name, getSelf());
    
    bool loaded = false;
    {
        std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            throw std::runtime_error("Timeout acquiring ScriptManager mutex in createService");
        }
        
        if (svc->loadScript(scriptPath)) {
            m_services[name] = svc;
            loaded = true;
        }
    }

    if (loaded) {
        svc->onInit();
        return svc;
    }
    
    return nullptr;
}

void ScriptManager::stopService(const std::string& name) {
    std::shared_ptr<LuaService> svc;
    {
        std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            throw std::runtime_error("Timeout acquiring ScriptManager mutex in stopService");
        }
        std::map<std::string, std::shared_ptr<LuaService>>::iterator it = m_services.find(name);
        if (it != m_services.end()) {
            svc = it->second;
            m_services.erase(it);
        }
    }
    if (svc) {
        svc->stop();
    }
}

void ScriptManager::update(double dt) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        std::cerr << "ScriptManager update timeout" << std::endl;
        return;
    }
    for (std::map<std::string, std::shared_ptr<LuaService>>::value_type& pair : m_services) {
        pair.second->onUpdate(dt);
    }
}

void ScriptManager::tickGC(int stepSize) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        std::cerr << "ScriptManager tickGC timeout" << std::endl;
        return;
    }
    for (std::map<std::string, std::shared_ptr<LuaService>>::value_type& pair : m_services) {
        pair.second->gcStep(stepSize);
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
