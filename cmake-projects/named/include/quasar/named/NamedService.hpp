#ifndef QUASAR_NAMED_NAMEDSERVICE_HPP
#define QUASAR_NAMED_NAMEDSERVICE_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedMethod.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace quasar::named {

/**
 * @class NamedService
 * @brief An active object that runs a background thread and uses NamedMethod hooks for its lifecycle.
 * 
 * NamedService can be used to encapsulate autonomous logic, such as fieldbus masters,
 * protocol handlers, or periodic tasks.
 * 
 * @reference [TSK-20260328-001] Reflexive Execution & Service Orchestration
 * @reference [FE-0260.3] Service Orchestration (NamedService)
 */
class NamedService : public NamedObject {
public:
    /**
     * @brief Factory method to create a new NamedService.
     * @param name The name of the service.
     * @param parent Optional parent.
     * @return A shared_ptr to the newly created NamedService.
     */
    static std::shared_ptr<NamedService> create(const std::string& name, std::shared_ptr<NamedObject> parent = nullptr);

    /**
     * @brief Destructor. Ensures the service is stopped.
     */
    virtual ~NamedService();

    /**
     * @brief Starts the service.
     * 
     * This will call the "configure" hook if it exists, then spawn the background thread.
     */
    virtual void start();

    /**
     * @brief Stops the service.
     * 
     * This will signal the background thread to stop and wait for its completion.
     */
    virtual void stop();

    /**
     * @brief Checks if the service is currently running.
     * @return true if running.
     */
    bool isRunning() const;

    /**
     * @brief Sets the cycle time for the "run" loop.
     * @param cycleTime The duration to sleep between loop iterations.
     */
    void setCycleTime(std::chrono::milliseconds cycleTime);

    /**
     * @brief Gets the type of the object.
     * @return "NamedService".
     */
    std::string getType() const override;

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the service.
     */
    NamedService(const std::string& name);

    /**
     * @brief The main loop of the background thread.
     */
    void serviceLoop();

    /**
     * @brief Helper to find and execute a NamedMethod child by name.
     * @param methodName The name of the method to look for.
     * @param args The arguments to pass.
     * @return The result of the execution, or nullptr if not found.
     */
    std::shared_ptr<NamedObject> callHook(const std::string& methodName, std::shared_ptr<NamedObject> args = nullptr);

private:
    /** @brief Atomic flag to control the thread loop. */
    std::atomic<bool> m_running{false};
    /** @brief Atomic flag indicating if the thread is active. */
    std::atomic<bool> m_threadActive{false};
    /** @brief The background thread. */
    std::thread m_thread;
    /** @brief The cycle time for the loop. */
    std::chrono::milliseconds m_cycleTime{100};
    /** @brief Condition variable for interruptible sleep. */
    std::condition_variable m_cv;
    /** @brief Mutex for the condition variable. */
    mutable std::mutex m_cvMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDSERVICE_HPP
