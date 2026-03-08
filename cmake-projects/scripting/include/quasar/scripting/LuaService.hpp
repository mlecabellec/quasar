#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/named/NamedObject.hpp"
#include <string>
#include <memory>
#include <chrono>

namespace quasar::scripting {

/**
 * @brief Interface for objects with a scriptable lifecycle.
 */
class ScriptComponent {
public:
    virtual ~ScriptComponent() = default;

    /**
     * @brief Initialization hook.
     * Called when the component is first loaded.
     */
    virtual bool onInit() = 0;

    /**
     * @brief Periodic update hook.
     * @param dt Delta time since last update.
     */
    virtual void onUpdate(double dt) = 0;

    /**
     * @brief Shutdown hook.
     * Called before the component is destroyed.
     */
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

    /**
     * @brief Loads and executes a Lua script file as the service body.
     * @param path Path to the .lua file.
     * @return True if loaded successfully.
     */
    bool loadScript(const std::string& path);

    /**
     * @brief Direct script execution within the service context.
     */
    sol::protected_function_result execute(const std::string& script);

    // ScriptComponent implementation
    bool onInit() override;
    void onUpdate(double dt) override;
    void onShutdown() override;

    /**
     * @brief Gets the engine used by this service.
     */
    std::shared_ptr<LuaEngine> getEngine() const { return m_engine; }

protected:
    LuaService(const std::string& name);

private:
    std::shared_ptr<LuaEngine> m_engine;
    sol::table m_luaSelf;
};

} // namespace quasar::scripting
