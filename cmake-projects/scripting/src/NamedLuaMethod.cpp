#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include <iostream>
#include <stdexcept>
#include <mutex>

namespace quasar::scripting {

/** @brief Extraction helper forward declaration. */
std::shared_ptr<quasar::named::NamedObject> extractNamedObject(sol::object obj);

NamedLuaMethod::NamedLuaMethod(const std::string& name, sol::function func)
    : NamedLuaMethod(name, std::make_shared<NamedLuaMethodImpl>()) 
{
    m_impl->func = func;
}

NamedLuaMethod::NamedLuaMethod(const std::string& name, std::shared_ptr<NamedLuaMethodImpl> impl)
    : NamedMethod(name, [impl](std::shared_ptr<quasar::named::NamedObject> owner, std::shared_ptr<quasar::named::NamedObject> args) -> std::shared_ptr<quasar::named::NamedObject> {
        std::unique_lock<std::recursive_mutex> implLock(impl->mutex);
        if (!impl->func.valid()) {
            return nullptr;
        }

        // [CS-0010.44] Retrieve the engine from the Lua registry to acquire the thread lock.
        sol::state_view lua(impl->func.lua_state());
        sol::object engineObj = lua.registry()["__quasar_engine"];
        std::unique_lock<std::recursive_mutex> lock;
        
        if (engineObj.is<LuaEngine*>()) {
            LuaEngine* engine = engineObj.as<LuaEngine*>();
            lock = engine->acquireLock();
        }

        // [CS-0010.44] Execute the Lua function with owner and args as proxies.
        sol::protected_function_result result = impl->func(LuaProxy<quasar::named::NamedObject>(owner), LuaProxy<quasar::named::NamedObject>(args));
        
        if (!result.valid()) {
            sol::error err = result;
            throw std::runtime_error("NamedLuaMethod execution error: " + std::string(err.what()));
        }
        
        return extractNamedObject(result);
    }), m_impl(impl) {}

NamedLuaMethod::~NamedLuaMethod() {
    // [CS-0010.44] Destructor should not throw.
    try {
        invalidate();
    } catch (...) {}
}

void NamedLuaMethod::invalidate() {
    if (m_impl) {
        std::unique_lock<std::recursive_mutex> lock(m_impl->mutex);
        if (m_impl->func.valid()) {
            // [CS-0010.44] Abandon the reference to avoid unreferencing during/after state destruction.
            m_impl->func.abandon();
        }
    }
}

std::shared_ptr<NamedLuaMethod> NamedLuaMethod::create(const std::string& name, sol::function func, std::shared_ptr<quasar::named::NamedObject> parent) {
    if (!func.valid()) {
        throw std::invalid_argument("NamedLuaMethod: Lua function is invalid");
    }
    // [CS-0010.10] Use of new or delete keywords is forbidden.
    struct make_shared_enabler : public NamedLuaMethod {
        explicit make_shared_enabler(const std::string& n, sol::function f) : NamedLuaMethod(n, f) {}
    };
    std::shared_ptr<NamedLuaMethod> self = std::make_shared<make_shared_enabler>(name, func);
    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }
    return self;
}

std::string NamedLuaMethod::getType() const {
    return "NamedLuaMethod";
}

} // namespace quasar::scripting
