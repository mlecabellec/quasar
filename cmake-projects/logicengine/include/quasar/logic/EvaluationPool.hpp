/**
 * @file EvaluationPool.hpp
 * @brief Threaded evaluation pool with watchdog for safe logic execution.
 */

#ifndef QUASAR_LOGIC_EVALUATIONPOOL_HPP
#define QUASAR_LOGIC_EVALUATIONPOOL_HPP

#include "quasar/named/NamedObject.hpp"
#include <sol/sol.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>

namespace quasar::logic {

/** @brief Status of a logic evaluation task. */
enum class EvaluationStatus {
    Pending,
    Success,    /**< Expression evaluated to true. */
    LogicFalse, /**< Expression evaluated to false. */
    Error,      /**< Runtime error (e.g. nil access). */
    Timeout     /**< Watchdog killed the execution. */
};

/** @brief Task for the evaluation pool. */
struct EvaluationTask {
    std::string bytecode;
    std::shared_ptr<quasar::named::NamedObject> contextRoot;
    
    std::mutex mutex;
    std::condition_variable cv;
    EvaluationStatus status{EvaluationStatus::Pending};
    bool completed{false};
};

/**
 * @class LogicWorker
 * @brief Individual thread managing one Lua state.
 */
class LogicWorker {
public:
    LogicWorker(std::size_t id);
    ~LogicWorker();

    void start(std::function<void(LogicWorker&)> mainLoop);
    
    /** @brief Signals the worker to cancel the current Lua execution. */
    void cancel();

    void setTask(std::shared_ptr<EvaluationTask> task);
    void clearTask();
    
    std::chrono::milliseconds getElapsed() const;
    sol::state& getLua() { return m_lua; }
    std::size_t getId() const { return m_id; }
    
    /** @brief Checks if a cancellation was requested. */
    bool isCancelled() const { return m_cancelRequested; }

private:
    std::size_t m_id;
    std::thread m_thread;
    sol::state m_lua;
    std::atomic<bool> m_busy{false};
    std::atomic<bool> m_cancelRequested{false};
    std::atomic<std::chrono::steady_clock::time_point> m_lastTaskStart;
    std::shared_ptr<EvaluationTask> m_currentTask;
    mutable std::mutex m_workerMutex;
};

/**
 * @class EvaluationPool
 * @brief Manages a pool of workers and monitors them for timeouts.
 */
class EvaluationPool {
public:
    static EvaluationPool& getInstance();

    /**
     * @brief Evaluates bytecode and waits for result.
     */
    EvaluationStatus evaluate(const std::vector<std::byte>& bytecode, std::shared_ptr<quasar::named::NamedObject> context);

    void setHardTimeout(std::chrono::milliseconds timeout) { m_hardTimeout = timeout; }
    void shutdown();

private:
    EvaluationPool();
    ~EvaluationPool();

    void workerLoop(LogicWorker& worker);
    void watchdogLoop();

    std::vector<std::unique_ptr<LogicWorker>> m_workers;
    std::queue<std::shared_ptr<EvaluationTask>> m_tasks;
    
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running{true};
    
    std::chrono::milliseconds m_hardTimeout{50};
    std::thread m_watchdogThread;

    void respawnWorker(std::size_t id);
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_EVALUATIONPOOL_HPP
