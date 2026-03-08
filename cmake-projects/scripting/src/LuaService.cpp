#include "quasar/scripting/LuaService.hpp"
#include <iostream>

namespace quasar::scripting {

std::shared_ptr<LuaService> LuaService::create(const std::string& name, std::shared_ptr<named::NamedObject> parent) {
    auto svc = std::shared_ptr<LuaService>(new LuaService(name));
    svc->setSelf(svc);
    if (parent) {
        svc->setParent(parent);
    }
    return svc;
}

LuaService::LuaService(const std::string& name) 
    : named::NamedObject(name), m_engine(std::make_shared<LuaEngine>()) {
    // Each service gets its own isolated engine.
}

bool LuaService::loadScript(const std::string& path) {
    try {
        auto result = m_engine->getState().script_file(path);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] failed to load script: " << err.what() << std::endl;
            return false;
        }
        
        // If the script returns a table, treat it as the service object
        if (result.return_count() > 0 && result[0].get_type() == sol::type::table) {
            m_luaSelf = result[0];
        } else {
            // Otherwise, look for global hooks in this engine's state
            m_luaSelf = m_engine->getState().globals();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "LuaService [" << getName() << "] exception during load: " << e.what() << std::endl;
        return false;
    }
}

sol::protected_function_result LuaService::execute(const std::string& script) {
    return m_engine->executeString(script);
}

bool LuaService::onInit() {
    if (m_luaSelf && m_luaSelf["onInit"].valid()) {
        sol::protected_function func = m_luaSelf["onInit"];
        auto result = func(m_luaSelf);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onInit error: " << err.what() << std::endl;
            return false;
        }
        return result.get_type() == sol::type::boolean ? result.get<bool>() : true;
    }
    return true;
}

void LuaService::onUpdate(double dt) {
    if (m_luaSelf && m_luaSelf["onUpdate"].valid()) {
        sol::protected_function func = m_luaSelf["onUpdate"];
        auto result = func(m_luaSelf, dt);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onUpdate error: " << err.what() << std::endl;
        }
    }
}

void LuaService::onShutdown() {
    if (m_luaSelf && m_luaSelf["onShutdown"].valid()) {
        sol::protected_function func = m_luaSelf["onShutdown"];
        auto result = func(m_luaSelf);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onShutdown error: " << err.what() << std::endl;
        }
    }
}

} // namespace quasar::scripting
