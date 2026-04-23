#pragma once

#include <sol/sol.hpp>
#include <string>
#include <vector>

namespace quasar::scripting {

/**
 * @brief Dynamic library loader for Quasar Scripting plugins.
 * 
 * Capable of taking a shared library path, resolving the registerPluginComponents
 * entry point, and invoking it securely. 
 * 
 * @reference [TSK-20260310-001.3] Dynamic Library Loading
 * @reference [FE-0140] Standalone Script Runner and Plug-in System
 */
class PluginLoader {
public:
    /**
     * @brief Loads a plugin from the given path and registers its components to the Lua state.
     * 
     * @param libraryPath Path to the shared library (.so, .dll, etc.).
     * @param lua The Lua state to pass to the plugin.
     * @return true if loading and registration succeeded, false otherwise.
     */
    static bool loadPlugin(const std::string& libraryPath, sol::state_view lua);

protected:
    // Internal generic library load routine
    static void* loadLibrary(const std::string& libraryPath);
    // Internal symbol resolution
    static void* getSymbolAddress(void* handle, const std::string& symbolName);
    // Unloading library, note: usually plugins stay loaded
    static void unloadLibrary(void* handle);

    // Keep handles open to prevent unloading code that we are bound to.
    static std::vector<void*> s_loadedHandles;
};

} // namespace quasar::scripting
