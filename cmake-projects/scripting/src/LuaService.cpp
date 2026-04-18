#include "quasar/scripting/LuaService.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <iostream>

namespace quasar::scripting {

std::shared_ptr<LuaService> LuaService::create(const std::string& name, std::shared_ptr<named::NamedObject> parent) {
    struct Enabler : public LuaService {
        explicit Enabler(const std::string& n) : LuaService(n) {}
    };
    std::shared_ptr<LuaService> svc = std::make_shared<Enabler>(name);
    svc->setSelf(svc);
    svc->m_engine = LuaEngine::create(svc);
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
    std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring LuaState mutex in loadScript");
    }
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
    std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring LuaState mutex in execute");
    }
    return m_engine->executeString(script);
}

void LuaService::start() {
    if (m_running) return;
    m_running = true;
    m_worker = std::thread(&LuaService::workerLoop, this);
}

void LuaService::stop() {
    if (!m_running) return;
    m_running = false;
    m_cv.notify_all();
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

bool LuaService::onInit() {
    bool initOk = true;

    {
        std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            throw std::runtime_error("Timeout acquiring LuaState mutex in onInit");
        }
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
    std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        // Log error but don't throw to avoid crashing the worker loop or system.
        std::cerr << "LuaService [" << getName() << "] timeout acquiring mutex in onUpdate" << std::endl;
        return;
    }
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

    std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        std::cerr << "LuaService [" << getName() << "] timeout acquiring mutex in onShutdown" << std::endl;
    } else if (m_luaSelf && m_luaSelf["onShutdown"].valid()) {
        sol::protected_function func = m_luaSelf["onShutdown"];
        sol::protected_function_result result = func(m_luaSelf);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "LuaService [" << getName() << "] onShutdown error: " << err.what() << std::endl;
        }
    }

    // [CS-0010.21] RAII: Ensure the engine is explicitly shut down before pointer resets.
    if (m_engine) {
        m_engine->shutdown();
    }
}

void LuaService::gcStep(int stepSize) {
    std::unique_lock<std::recursive_timed_mutex> lock(m_stateMutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
         std::cerr << "LuaService [" << getName() << "] timeout acquiring mutex in gcStep" << std::endl;
         return;
    }
    if (m_engine) {
        m_engine->gcStep(stepSize);
    }
}

void LuaService::workerLoop() {
    std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();
    while (m_running) {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - lastUpdate).count();
        lastUpdate = now;

        // 1. Regular Update
        onUpdate(dt);

        // 2. Process Asynchronous Tasks (Events)
        std::unique_lock<std::timed_mutex> lock(m_queueMutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
             std::cerr << "LuaService [" << getName() << "] workerLoop timeout acquiring queue mutex" << std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(10));
             continue;
        }
        m_cv.wait_for(lock, std::chrono::milliseconds(10), [this] { 
            return !m_running || !m_taskQueue.empty(); 
        });

        while (!m_taskQueue.empty() && m_running) {
            std::function<void()> task = std::move(m_taskQueue.front());
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
        std::unique_lock<std::timed_mutex> lock(m_queueMutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) {
            throw std::runtime_error("Timeout acquiring queue mutex in postTask");
        }
        m_taskQueue.push(std::move(task));
    }
    m_cv.notify_one();
}

} // namespace quasar::scripting
