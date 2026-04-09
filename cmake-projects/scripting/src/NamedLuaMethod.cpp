#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaService.hpp"
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <future>

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
        
        std::function<std::shared_ptr<quasar::named::NamedObject>()> executeInternal = [impl, owner, args]() -> std::shared_ptr<quasar::named::NamedObject> {
            if (!impl->valid || !impl->engine) return nullptr;

            // [CS-0010.46] Always acquire engine lock before touching the state from any thread.
            std::unique_lock<std::recursive_mutex> lock = impl->engine->acquireLock();

            std::unique_lock<std::recursive_mutex> implLock(impl->mutex);
            if (!impl->func.valid() || !impl->valid) {
                return nullptr;
            }

            sol::protected_function_result result = impl->func(LuaProxy<quasar::named::NamedObject>(owner), LuaProxy<quasar::named::NamedObject>(args));
            
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "NamedLuaMethod [" << owner->getName() << "] execution error: " << err.what() << std::endl;
                return nullptr;
            }
            
            return extractNamedObject(result);
        };

        std::shared_ptr<LuaService> svc = impl->service.lock();
        if (svc) {
            if (svc->isRunning()) {
                std::future<std::shared_ptr<quasar::named::NamedObject>> future = svc->postTaskWithResult<std::shared_ptr<quasar::named::NamedObject>>(executeInternal);
                if (future.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
                    return future.get();
                } else {
                    return nullptr;
                }
            }
        }

        return executeInternal();
    }), m_impl(impl) {}

NamedLuaMethod::~NamedLuaMethod() {
    invalidate();
}

void NamedLuaMethod::invalidate() {
    if (m_impl) {
        m_impl->valid = false;
        std::unique_lock<std::recursive_mutex> lock(m_impl->mutex);
        if (m_impl->func.valid()) {
            m_impl->func.abandon();
        }
    }
}

std::shared_ptr<NamedLuaMethod> NamedLuaMethod::create(const std::string& name, sol::function func, std::shared_ptr<quasar::named::NamedObject> parent) {
    if (!func.valid()) {
        throw std::invalid_argument("NamedLuaMethod: Lua function is invalid");
    }
    struct make_shared_enabler : public NamedLuaMethod {
        explicit make_shared_enabler(const std::string& n, sol::function f) : NamedLuaMethod(n, f) {}
    };
    std::shared_ptr<NamedLuaMethod> self = std::make_shared<make_shared_enabler>(name, func);
    self->setSelf(self);
    
    // Setup LuaEngine pointer
    sol::state_view lua(func.lua_state());
    sol::object engineObj = lua["__quasar_engine"];
    if (engineObj.is<LuaEngine*>()) {
        self->m_impl->engine = engineObj.as<LuaEngine*>();
    }
    
    std::shared_ptr<quasar::named::NamedObject> p = parent;
    while (p) {
        std::shared_ptr<LuaService> svc = std::dynamic_pointer_cast<LuaService>(p);
        if (svc) {
            self->m_impl->service = svc;
            break;
        }
        p = p->getParent();
    }

    if (parent) {
        self->setParent(parent);
    }
    return self;
}

std::string NamedLuaMethod::getType() const {
    return "NamedLuaMethod";
}

} // namespace quasar::scripting
