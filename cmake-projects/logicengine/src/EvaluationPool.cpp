/**
 * @file EvaluationPool.cpp
 * @brief Implementation of the threaded evaluation pool and watchdog.
 */

#include "quasar/logic/EvaluationPool.hpp"
#include "quasar/logic/Expression.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include <pthread.h>
#include <signal.h>
#include <iostream>

namespace quasar::logic {

static std::mutex s_workersMutex;
static thread_local LogicWorker* s_currentWorker = nullptr;

// Lua hook function
static void luaInstructionHook(lua_State* L, lua_Debug* /*ar*/) {
    if (s_currentWorker && s_currentWorker->isCancelled()) {
        luaL_error(L, "Script execution timeout (Watchdog killed)");
    }
}

// --- LogicWorker Implementation ---

LogicWorker::LogicWorker(std::size_t id) : m_id(id) {
    // [CS-0010.44] Open standard libraries and register once.
    m_lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::bit32);
    
    // Stabilize usertypes and store worker identity.
    quasar::logic::registerLogicTypes(m_lua);
    m_lua["__logic_worker_id"] = m_id;

    // Safety watchdog hook.
    lua_sethook(m_lua.lua_state(), luaInstructionHook, LUA_MASKCOUNT, 1000);
}

LogicWorker::~LogicWorker() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void LogicWorker::start(std::function<void(LogicWorker&)> mainLoop) {
    m_thread = std::thread([this, mainLoop]() {
        s_currentWorker = this;
        mainLoop(*this);
    });
}

void LogicWorker::cancel() {
    m_cancelRequested = true;
}

void LogicWorker::setTask(std::shared_ptr<EvaluationTask> task) {
    std::lock_guard<std::mutex> lock(m_workerMutex);
    m_currentTask = std::move(task);
    m_cancelRequested = false;
    m_lastTaskStart = std::chrono::steady_clock::now();
    m_busy = true;
}

void LogicWorker::clearTask() {
    std::lock_guard<std::mutex> lock(m_workerMutex);
    m_currentTask.reset();
    m_busy = false;
    m_cancelRequested = false;
}

std::chrono::milliseconds LogicWorker::getElapsed() const {
    if (!m_busy) return std::chrono::milliseconds(0);
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_lastTaskStart.load());
}

// --- EvaluationPool Implementation ---

EvaluationPool& EvaluationPool::getInstance() {
    static EvaluationPool instance;
    return instance;
}

EvaluationPool::EvaluationPool() {
    const std::size_t numWorkers = std::max(1u, std::thread::hardware_concurrency());
    for (std::size_t i = 0; i < numWorkers; ++i) {
        respawnWorker(i);
    }
    m_watchdogThread = std::thread(&EvaluationPool::watchdogLoop, this);
}

EvaluationPool::~EvaluationPool() {
    shutdown();
}

void EvaluationPool::respawnWorker(std::size_t id) {
    std::lock_guard<std::mutex> lock(s_workersMutex);
    std::unique_ptr<LogicWorker> worker = std::make_unique<LogicWorker>(id);
    LogicWorker* rawPtr = worker.get();
    worker->start([this, rawPtr](LogicWorker& /*w*/) { workerLoop(*rawPtr); });
    
    if (id < m_workers.size()) {
        m_workers[id] = std::move(worker);
    } else {
        m_workers.push_back(std::move(worker));
    }
}

EvaluationStatus EvaluationPool::evaluate(const std::vector<std::byte>& bytecode, std::shared_ptr<quasar::named::NamedObject> context) {
    std::shared_ptr<EvaluationTask> task = std::make_shared<EvaluationTask>();
    task->bytecode.assign(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
    task->contextRoot = std::move(context);
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_tasks.push(task);
    }
    m_cv.notify_one();

    std::unique_lock<std::mutex> taskLock(task->mutex);
    task->cv.wait(taskLock, [&task] { return task->completed; });
    return task->status;
}

void EvaluationPool::workerLoop(LogicWorker& worker) {
    while (m_running) {
        std::shared_ptr<EvaluationTask> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] { return !m_tasks.empty() || !m_running; });
            if (!m_running && m_tasks.empty()) break;
            task = m_tasks.front();
            m_tasks.pop();
        }

        worker.setTask(task);
        EvaluationStatus finalStatus = EvaluationStatus::Error;
        
        try {
            Expression::bindContext(worker.getLua(), task->contextRoot);

            sol::load_result loadResult = worker.getLua().load(task->bytecode);
            if (loadResult.valid()) {
                sol::protected_function func = loadResult;
                sol::protected_function_result result = func();
                if (result.valid()) {
                    finalStatus = result.get<bool>() ? EvaluationStatus::Success : EvaluationStatus::LogicFalse;
                } else {
                    sol::error err = result;
                    std::cout << "[LUA ERROR] " << err.what() << std::endl;
                    if (std::string(err.what()).find("Watchdog killed") != std::string::npos) {
                        finalStatus = EvaluationStatus::Timeout;
                    }
                }
            }
        } catch (...) {
            // Error fallback
        }
        
        // [CS-0010.21] Cleanup contextual strong references created during evaluation.
        // This worker ID is used to partition the ObjectTracker.
        quasar::scripting::ObjectTracker::getInstance().untrackAll(worker.getId());

        if (worker.isCancelled() && finalStatus == EvaluationStatus::Error) {
            finalStatus = EvaluationStatus::Timeout;
        }

        {
            std::lock_guard<std::mutex> taskLock(task->mutex);
            if (!task->completed) {
                task->status = finalStatus;
                task->completed = true;
                task->cv.notify_all();
            }
        }
        
        worker.clearTask();
    }
}

void EvaluationPool::watchdogLoop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::lock_guard<std::mutex> lock(s_workersMutex);
        for (std::size_t i = 0; i < m_workers.size(); ++i) {
            if (m_workers[i] && m_workers[i]->getElapsed() > m_hardTimeout) {
                m_workers[i]->cancel();
            }
        }
    }
}

void EvaluationPool::shutdown() {
    m_running = false;
    m_cv.notify_all();
    if (m_watchdogThread.joinable()) {
        m_watchdogThread.join();
    }
    std::lock_guard<std::mutex> lock(s_workersMutex);
    m_workers.clear();
}

} // namespace quasar::logic
