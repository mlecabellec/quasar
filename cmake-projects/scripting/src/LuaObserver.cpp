#include "quasar/scripting/LuaObserver.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include <iostream>

namespace quasar::scripting {

LuaObserver::LuaObserver(sol::function func, LuaEngine* engine, std::shared_ptr<LuaService> service)
    : m_func(func), m_engine(engine), m_service(service) {}

LuaObserver::~LuaObserver() {
    invalidate();
}

void LuaObserver::notify(std::shared_ptr<quasar::named::NamedObject> eventData) {
    std::function<void()> notifyInternal = [this, eventData]() {
        if (m_valid == false || m_engine == nullptr) return;

        // [CS-0010.46] Always acquire engine lock.
        std::unique_lock<std::recursive_mutex> lock = m_engine->acquireLock();

        std::unique_lock<std::recursive_mutex> implLock(m_mutex);
        if (m_func.valid() == false || m_valid == false) {
            return;
        }

        sol::protected_function_result result = m_func(LuaProxy<quasar::named::NamedObject>(eventData));
        if (result.valid() == false) {
            sol::error err = result;
            std::cerr << "LuaObserver notification error: " << err.what() << std::endl;
        }
    };

    std::shared_ptr<LuaService> svc = m_service.lock();
    if (svc != nullptr) {
        if (svc->isRunning() == true) {
            svc->postTask(notifyInternal);
            return;
        }
    }

    notifyInternal();
}

void LuaObserver::invalidate() {
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_valid = false;
    if (m_func.valid()) {
        m_func.abandon();
    }
}

} // namespace quasar::scripting
