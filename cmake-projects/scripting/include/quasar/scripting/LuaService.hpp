#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <future>
#include <functional>

namespace quasar::scripting {

/**
 * @brief Interface for objects with a scriptable lifecycle.
 */
class ScriptComponent {
public:
    virtual ~ScriptComponent() = default;

    /// @brief Initialization hook. Called when the component is first loaded.
    virtual bool onInit() = 0;

    /// @brief Periodic update hook.
    /// @param dt Delta time since last update.
    virtual void onUpdate(double dt) = 0;

    /// @brief Shutdown hook. Called before the component is destroyed.
    virtual void onShutdown() = 0;
};

/**
 * @brief A persistent, stateful Lua service.
 * 
 * LuaService owns its own LuaEngine and can run long-lived scripts
 * that respond to system events or periodic updates.
 */
class LuaService : public named::NamedObject, public ScriptComponent {
public:
    /**
     * @brief Factory method.
     */
    static std::shared_ptr<LuaService> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /// @brief Loads and executes a Lua script file as the service body.
    /// @param path Path to the .lua file.
    bool loadScript(const std::string& path);

    /**
     * @brief Direct script execution within the service context.
     */
    sol::protected_function_result execute(const std::string& script);

    /// @brief Initialize the service components.
    bool onInit() override;
    /// @brief Periodic update callback for the service.
    void onUpdate(double dt) override;
    /// @brief Shutdown callback for the service.
    void onShutdown() override;

    /**
     * @brief Starts the service background thread.
     */
    void start();

    /**
     * @brief Stops the service and joins the thread.
     */
    void stop();

    /**
     * @brief Gets the engine used by this service.
     */
    std::shared_ptr<LuaEngine> getEngine() const { return m_engine; }

    /**
     * @brief Posts a task to be executed on the service's worker thread.
     */
    void postTask(std::function<void()> task);

    /// @brief Posts a task to the service worker thread and returns a future for the result.
    template<typename T>
    std::future<T> postTaskWithResult(std::function<T()> task) {
        std::shared_ptr<std::promise<T>> promise = std::make_shared<std::promise<T>>();
        postTask([task, promise]() {
            try {
                promise->set_value(task());
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        });
        return promise->get_future();
    }

    /**
     * @brief Check if the service is currently running.
     */
    bool isRunning() const { return m_running; }

    /**
     * @brief Performs a thread-safe garbage collection step on the Lua engine.
     */
    void gcStep(int stepSize);

    /**
     * @brief Exposes the state mutex for manual synchronization.
     */
    std::recursive_timed_mutex& getStateMutex() { return m_stateMutex; }

protected:
    LuaService(const std::string& name);

protected:
    std::shared_ptr<LuaEngine> m_engine;
    sol::table m_luaSelf;
    std::recursive_timed_mutex m_stateMutex;

    // Threading
    std::thread m_worker;
    std::atomic<bool> m_running{false};
    std::condition_variable_any m_cv;
    std::timed_mutex m_queueMutex;
    std::queue<std::function<void()>> m_taskQueue;

    /// @brief Main worker thread execution loop.
    void workerLoop();
};

} // namespace quasar::scripting
