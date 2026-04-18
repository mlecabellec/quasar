#include "quasar/scripting/ScriptExecutor.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaService.hpp"
#include <iostream>

namespace quasar::scripting {

void ScriptExecutor::ExecuteOnce(const std::string& script) {
    try {
        // [CS-0010.6] Use shared_ptr via factory for temporary engine.
        std::shared_ptr<LuaEngine> engine = LuaEngine::create();
        engine->executeString(script);
        // Explicit shutdown to clear Lua registry.
        engine->shutdown();
    } catch (const std::exception& e) {
        std::cerr << "ScriptExecutor::ExecuteOnce error: " << e.what() << std::endl;
    }
}

void ScriptExecutor::ExecuteSync(const std::string& script, std::shared_ptr<LuaService> service) {
    if (!service) return;
    // Post to service thread and wait.
    service->postTaskWithResult<bool>([service, script]() {
        service->getEngine()->executeString(script);
        return true;
    }).get();
}

void ScriptExecutor::ExecuteAsync(const std::string& script, std::shared_ptr<LuaService> service) {
    if (!service) return;
    // Capture engine by shared_ptr to ensure it stays alive for the async task.
    std::shared_ptr<LuaEngine> engine = service->getEngine();
    service->postTask([engine, script]() {
        engine->executeString(script);
    });
}

} // namespace quasar::scripting
