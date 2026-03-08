#pragma once

#include "quasar/scripting/LuaService.hpp"
#include "quasar/named/NamedObject.hpp"
#include <map>
#include <string>
#include <memory>
#include <mutex>

namespace quasar::scripting {

/**
 * @brief Orchestrates multiple Lua services and enforces sandboxing.
 * 
 * ScriptManager is the central entry point for managing persistent scripts.
 * it handles service creation, lifecycle updates, and secure sandboxing.
 */
class ScriptManager : public named::NamedObject {
public:
    static ScriptManager& getInstance();

    /**
     * @brief Creates and starts a new managed service.
     */
    std::shared_ptr<LuaService> createService(const std::string& name, const std::string& scriptPath);

    /**
     * @brief Removes a managed service by name.
     */
    void stopService(const std::string& name);

    /**
     * @brief Periodic update for all managed services.
     */
    void update(double dt);

    /**
     * @brief Performs a incremental GC step across all engines.
     * Prevents large GC pauses in real-time industrial contexts.
     */
    void tickGC(int stepSize = 100);

    /**
     * @brief Creates a restricted Lua environment (sandbox) for a state.
     */
    static sol::environment createSandbox(sol::state_view lua);

private:
    ScriptManager();
    
    std::mutex m_mutex;
    std::map<std::string, std::shared_ptr<LuaService>> m_services;
};

} // namespace quasar::scripting
