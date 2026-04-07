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
        
        auto executeInternal = [impl, owner, args]() -> std::shared_ptr<quasar::named::NamedObject> {
            std::unique_lock<std::recursive_mutex> implLock(impl->mutex);
            if (!impl->func.valid()) {
                return nullptr;
            }

            sol::state_view lua(impl->func.lua_state());
            sol::object engineObj = lua.registry()["__quasar_engine"];
            std::unique_lock<std::recursive_mutex> lock;
            
            if (engineObj.is<LuaEngine*>()) {
                LuaEngine* engine = engineObj.as<LuaEngine*>();
                lock = engine->acquireLock();
            }

            sol::protected_function_result result = impl->func(LuaProxy<quasar::named::NamedObject>(owner), LuaProxy<quasar::named::NamedObject>(args));
            
            if (!result.valid()) {
                sol::error err = result;
                throw std::runtime_error("NamedLuaMethod execution error: " + std::string(err.what()));
            }
            
            return extractNamedObject(result);
        };

        // Check if we need to marshal to a different thread
        if (auto svc = impl->service.lock()) {
            // Marshall to the service's worker thread
            auto future = svc->postTaskWithResult<std::shared_ptr<quasar::named::NamedObject>>(executeInternal);
            
            // Wait for completion. 
            // NOTE: If we are calling from the same thread, this might deadlock if not careful.
            // But LuaService worker thread doesn't typically call its own methods synchronously via this path.
            if (future.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
                return future.get();
            } else {
                throw std::runtime_error("NamedLuaMethod execution timed out (marshalling)");
            }
        } else {
            // No associated service (or calling from a thread that owns the state)
            return executeInternal();
        }
    }), m_impl(impl) {}

NamedLuaMethod::~NamedLuaMethod() {
    try {
        invalidate();
    } catch (...) {}
}

void NamedLuaMethod::invalidate() {
    if (m_impl) {
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
    
    // Associate with parent if it's a LuaService or under one
    std::shared_ptr<quasar::named::NamedObject> p = parent;
    while (p) {
        if (auto svc = std::dynamic_pointer_cast<LuaService>(p)) {
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
