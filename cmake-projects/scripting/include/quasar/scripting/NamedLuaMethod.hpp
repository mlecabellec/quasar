#ifndef QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP
#define QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP

#include "quasar/named/NamedMethod.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <mutex>
#include <atomic>

namespace quasar::scripting {

class LuaService;
class LuaEngine;

/**
 * @struct NamedLuaMethodImpl
 * @brief Internal implementation for NamedLuaMethod to allow safe invalidation.
 */
struct NamedLuaMethodImpl {
    /** @brief The bound Lua function. */
    sol::function func;
    /** @brief Mutex protecting the implementation state. */
    std::recursive_mutex mutex;
    /** @brief Reference to the host service if any. */
    std::weak_ptr<LuaService> service;
    /** @brief Weak reference to the engine to detect destruction. */
    std::weak_ptr<LuaEngine> engine;
    /** @brief Stored ID of the engine for post-destruction tracking. */
    size_t engineId{0};
    /** @brief Validity flag for safe shutdown. */
    std::atomic<bool> valid{true};
};


/**
 * @class NamedLuaMethod
 * @brief A NamedMethod that executes a Lua function.
 * 
 * @reference [TSK-20260328-001] Reflexive Execution & Service Orchestration
 * @feature [FE-0260.2] Scriptable Methods (NamedLuaMethod)
 */
class NamedLuaMethod : public quasar::named::NamedMethod {
public:
    /**
     * @brief Factory method to create a new NamedLuaMethod.
     * @param name The name of the method.
     * @param func The Lua function to execute.
     * @param parent Optional parent.
     * @return A shared_ptr to the newly created NamedLuaMethod.
     * @feature [FE-0260.2]
     * @exposed
     */
    static std::shared_ptr<NamedLuaMethod> create(const std::string& name, sol::function func, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~NamedLuaMethod();

    /**
     * @brief Returns the type of the object.
     * @return "NamedLuaMethod".
     * @exposed
     */
    [[nodiscard]] std::string getType() const override;

    /**
     * @brief Executes the method.
     * @param args Arguments for the execution.
     * @return Execution result.
     * @feature [FE-0260.2]
     * @exposed
     */
    std::shared_ptr<quasar::named::NamedObject> execute(std::shared_ptr<quasar::named::NamedObject> args) override;

    /**
     * @brief Invalidates the method by clearing the Lua function reference.
     * This is used during engine shutdown.
     */
    void invalidate();

    /**
     * @brief Gets the associated Lua engine.
     * @return Shared pointer to the engine, or nullptr if invalidated.
     * @exposed
     */
    [[nodiscard]] std::shared_ptr<LuaEngine> getEngine() const { return m_impl->engine.lock(); }

    /**
     * @brief Gets the ID of the associated Lua engine.
     * @return The engine ID.
     * @exposed
     */
    [[nodiscard]] size_t getEngineId() const { return m_impl->engineId; }

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the method.
     * @param func The Lua function.
     */
    NamedLuaMethod(const std::string& name, sol::function func);

    /** @brief Internal constructor for sharing Impl. */
    NamedLuaMethod(const std::string& name, std::shared_ptr<NamedLuaMethodImpl> impl);

private:
    std::shared_ptr<NamedLuaMethodImpl> m_impl;
};

} // namespace quasar::scripting

#endif // QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP
