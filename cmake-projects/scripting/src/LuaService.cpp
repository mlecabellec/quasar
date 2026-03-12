#include "quasar/scripting/LuaService.hpp"
#include <iostream>

namespace quasar::scripting {

std::shared_ptr<LuaService> LuaService::create(const std::string& name, std::shared_ptr<named::NamedObject> parent) {
    struct Enabler : public LuaService {
        Enabler(const std::string& n) : LuaService(n) {}
    };
    std::shared_ptr<LuaService> svc = std::make_shared<Enabler>(name);
    svc->setSelf(svc);
    svc->m_engine = std::make_shared<LuaEngine>(svc);
    if (parent) {
        svc->setParent(parent);
    }
    return svc;
}

LuaService::LuaService(const std::string& name) 
    : named::NamedObject(name) {
    // Delay engine creation until shared_from_this() is available via setSelf in create()
}

bool LuaService::loadScript(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    try {
        sol::protected_function_result result = m_engine->getState().script_file(path);
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
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    return m_engine->executeString(script);
}

bool LuaService::onInit() {
    bool initOk = true;

    {
        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
        if (m_luaSelf && m_luaSelf["onInit"].valid()) {
            sol::protected_function func = m_luaSelf["onInit"];
            sol::protected_function_result result = func(m_luaSelf);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "LuaService [" << getName() << "] onInit error: " << err.what() << std::endl;
                return false;
            }
            initOk = result.get_type() == sol::type::boolean ? result.get<bool>() : true;
        }
    }

    if (initOk) {
        m_running = true;
        m_worker = std::thread(&LuaService::workerLoop, this);
    }
    return initOk;
}

void LuaService::onUpdate(double dt) {
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    if (m_luaSelf && m_luaSelf["onUpdate"].valid()) {
        sol::protected_function func = m_luaSelf["onUpdate"];
        sol::protected_function_result result = func(m_luaSelf, dt);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onUpdate error: " << err.what() << std::endl;
        }
    }
}

void LuaService::onShutdown() {
    m_running = false;
    m_cv.notify_all();
    if (m_worker.joinable()) {
        m_worker.join();
    }

    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    if (m_luaSelf && m_luaSelf["onShutdown"].valid()) {
        sol::protected_function func = m_luaSelf["onShutdown"];
        sol::protected_function_result result = func(m_luaSelf);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onShutdown error: " << err.what() << std::endl;
        }
    }
}

void LuaService::gcStep(int stepSize) {
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    if (m_engine) {
        m_engine->gcStep(stepSize);
    }
}

void LuaService::workerLoop() {
    auto lastUpdate = std::chrono::steady_clock::now();
    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - lastUpdate).count();
        lastUpdate = now;

        // 1. Regular Update
        onUpdate(dt);

        // 2. Process Asynchronous Tasks (Events)
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_cv.wait_for(lock, std::chrono::milliseconds(10), [this] { 
            return !m_running || !m_taskQueue.empty(); 
        });

        while (!m_taskQueue.empty() && m_running) {
            auto task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
            lock.unlock();
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "LuaService [" << getName() << "] task exception: " << e.what() << std::endl;
            }
            lock.lock();
        }
    }
}

void LuaService::postTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push(std::move(task));
    }
    m_cv.notify_one();
}

} // namespace quasar::scripting
