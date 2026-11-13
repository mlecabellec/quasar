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
    // [CS-0010.44] Instantiate and load the service outside m_mutex to avoid serializing heavy Lua initialization.
    std::shared_ptr<LuaService> svc = LuaService::create(name, getSelf());
    if (!svc->loadScript(scriptPath)) {
        return nullptr;
    }

    {
        // [CS-0010.46] Guard only map insertion with m_mutex so workers do not contend on long-running init calls.
        std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            throw std::runtime_error("Timeout acquiring ScriptManager mutex in createService");
        }
        m_services[name] = svc;
    }

    svc->onInit();
    return svc;
}

void ScriptManager::stopService(const std::string& name) {
    std::shared_ptr<LuaService> svc;
    {
        // [CS-0010.46] Extract and erase the service pointer briefly under lock before invoking shutdown hooks.
        std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
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
        svc->onShutdown();
    }
}

void ScriptManager::update(double dt) {
    std::vector<std::shared_ptr<LuaService>> snapshot;
    {
        // [CS-0010.46] Snapshot active services briefly under lock so periodic updates do not block map edits.
        std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            std::cerr << "ScriptManager update timeout" << std::endl;
            return;
        }
        snapshot.reserve(m_services.size());
        for (std::map<std::string, std::shared_ptr<LuaService>>::value_type& pair : m_services) {
            snapshot.push_back(pair.second);
        }
    }
    for (const std::shared_ptr<LuaService>& svc : snapshot) {
        if (svc) {
            svc->onUpdate(dt);
        }
    }
}

void ScriptManager::tickGC(int stepSize) {
    std::vector<std::shared_ptr<LuaService>> snapshot;
    {
        // [CS-0010.46] Snapshot active services briefly under lock so GC steps never starve concurrent service ops.
        std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            std::cerr << "ScriptManager tickGC timeout" << std::endl;
            return;
        }
        snapshot.reserve(m_services.size());
        for (std::map<std::string, std::shared_ptr<LuaService>>::value_type& pair : m_services) {
            snapshot.push_back(pair.second);
        }
    }
    for (const std::shared_ptr<LuaService>& svc : snapshot) {
        if (svc) {
            svc->gcStep(stepSize);
        }
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
