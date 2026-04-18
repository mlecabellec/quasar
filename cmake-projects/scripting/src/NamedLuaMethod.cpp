#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include <iostream>
#include <stdexcept>
#include <future>

namespace quasar::scripting {

NamedLuaMethod::NamedLuaMethod(const std::string& name, std::shared_ptr<NamedLuaMethodImpl> impl)
    : NamedMethod(name, nullptr), m_impl(impl) {}

NamedLuaMethod::~NamedLuaMethod() {
    invalidate();
}

std::shared_ptr<quasar::named::NamedObject> NamedLuaMethod::execute(std::shared_ptr<quasar::named::NamedObject> args) {
    std::shared_ptr<NamedLuaMethodImpl> impl = m_impl;
    std::shared_ptr<quasar::named::NamedObject> owner = getParent();
    if (!owner) owner = getSelf();

    // Context-aware execution closure.
    std::function<std::shared_ptr<quasar::named::NamedObject>()> executeInternal = [impl, owner, args]() -> std::shared_ptr<quasar::named::NamedObject> {
        if (!impl->valid) return nullptr;
        
        // [CS-0010.6] Lock the engine to ensure it hasn't been destroyed.
        std::shared_ptr<LuaEngine> engine = impl->engine.lock();
        if (!engine) return nullptr;

        // [CS-0010.46] Always acquire engine lock before touching the state from any thread.
        std::unique_lock<std::recursive_mutex> lock = engine->acquireLock();

        std::unique_lock<std::recursive_mutex> implLock(impl->mutex);
        if (!impl->func.valid() || !impl->valid) {
            return nullptr;
        }

        // Call the Lua function with proxies.
        sol::protected_function_result result = impl->func(LuaProxy<quasar::named::NamedObject>(owner), LuaProxy<quasar::named::NamedObject>(args));

        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "NamedLuaMethod [" << owner->getName() << "] execution error: " << err.what() << std::endl;
            return nullptr;
        }

        // --- Handle diverse return types ---
        sol::object resObj = result;
        
        // 1. Existing NamedObject Proxy
        if (std::shared_ptr<quasar::named::NamedObject> p = extractNamedObject(resObj)) {
            return p;
        }
        
        // 2. Primitive Returns (Automatic wrapping)
        if (resObj.is<int64_t>()) return quasar::named::NamedInteger<int64_t>::create("result", resObj.as<int64_t>());
        if (resObj.is<double>()) return quasar::named::NamedFloatingPoint<double>::create("result", resObj.as<double>());
        if (resObj.is<bool>()) return quasar::named::NamedBoolean::create("result", resObj.as<bool>());
        if (resObj.is<std::string>()) return quasar::named::NamedString::create("result", resObj.as<std::string>());

        return nullptr;
    };

    // Routing execution to the service thread if available.
    std::shared_ptr<LuaService> svc = impl->service.lock();
    if (svc && svc->isRunning()) {
        std::future<std::shared_ptr<quasar::named::NamedObject>> future = svc->postTaskWithResult<std::shared_ptr<quasar::named::NamedObject>>(executeInternal);
        // Industrial timeout to prevent hanging on stalled scripts.
        if (future.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
            return future.get();
        } else {
            std::cerr << "NamedLuaMethod execution TIMEOUT" << std::endl;
            return nullptr;
        }
    }

    // Direct synchronous fallback (unsafe if called from multiple threads without service).
    return executeInternal();
}

void NamedLuaMethod::invalidate() {
    std::unique_lock<std::recursive_mutex> lock(m_impl->mutex);
    m_impl->valid = false;
    m_impl->func = sol::nil;
}

std::shared_ptr<NamedLuaMethod> NamedLuaMethod::create(const std::string& name, sol::function func, std::shared_ptr<quasar::named::NamedObject> parent) {
    std::shared_ptr<NamedLuaMethodImpl> impl = std::make_shared<NamedLuaMethodImpl>();
    impl->func = func;
    
    struct Enabler : public NamedLuaMethod {
        Enabler(const std::string& n, std::shared_ptr<NamedLuaMethodImpl> i) : NamedLuaMethod(n, i) {}
    };
    std::shared_ptr<Enabler> self = std::make_shared<Enabler>(name, impl);
    self->setSelf(self);

    // Setup LuaEngine and Service context.
    sol::state_view lua(func.lua_state());
    sol::object engineObj = lua["__quasar_engine"];
    if (engineObj.is<LuaEngine*>()) {
        LuaEngine* rawEngine = engineObj.as<LuaEngine*>();
        self->m_impl->engine = rawEngine->shared_from_this();
    }
    
    std::shared_ptr<quasar::named::NamedObject> p = parent;
    while (p) {
        if (std::shared_ptr<LuaService> svc = std::dynamic_pointer_cast<LuaService>(p)) {
            self->m_impl->service = svc;
            break;
        }
        p = p->getParent();
    }

    if (parent) {
        self->setParent(parent);
    }

    ObjectTracker::getInstance().track(self);
    return self;
}

std::string NamedLuaMethod::getType() const {
    return "NamedLuaMethod";
}

} // namespace quasar::scripting
