#pragma once

#include <sol/sol.hpp>

// Cross-platform export macro
#if defined(_WIN32)
    #define QUASAR_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define QUASAR_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    /**
     * @brief The standard entry point that a plugin shared library MUST implement.
     * 
     * The Quasar scripting runner will load the shared library and look for this symbol.
     * When found, it will be invoked with a view to the runner's internal Lua state,
     * allowing the plugin to register its own types, functions, and ScriptableNamedObjects.
     * 
     * @details Contributes to [FE-0140].
     * @param lua A sol::state_view into the execution environment.
     */
    QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua);
}
