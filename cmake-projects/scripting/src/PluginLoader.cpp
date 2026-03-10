#include "quasar/scripting/PluginLoader.hpp"
#include <iostream>

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace quasar::scripting {

/**
 * @details Implements the logic to interface with the OS dynamic linker.
 *          Contributes to [FE-0140].
 */
std::vector<void*> PluginLoader::s_loadedHandles;

bool PluginLoader::loadPlugin(const std::string& libraryPath, sol::state_view lua) {
    void* handle = loadLibrary(libraryPath);
    if (!handle) {
        std::cerr << "Failed to load plugin library: " << libraryPath << std::endl;
        return false;
    }

    void* symbol = getSymbolAddress(handle, "registerPluginComponents");
    if (!symbol) {
        std::cerr << "Plugin library " << libraryPath << " does not export registerPluginComponents" << std::endl;
        return false;
    }

    using RegisterFunc = void(*)(sol::state_view);
    RegisterFunc registerFunc = reinterpret_cast<RegisterFunc>(symbol);

    try {
        registerFunc(lua);
    } catch (const std::exception& e) {
        std::cerr << "Exception during plugin registration: " << e.what() << std::endl;
        return false;
    }

    s_loadedHandles.push_back(handle);
    return true;
}

void* PluginLoader::loadLibrary(const std::string& libraryPath) {
#if defined(_WIN32) || defined(__WIN32__)
    HMODULE handle = ::LoadLibraryA(libraryPath.c_str());
    if (!handle) {
        std::cerr << "Failed to load library on Windows: " << libraryPath << std::endl;
    }
    return static_cast<void*>(handle);
#else
    void* handle = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle && libraryPath.find(".so") == std::string::npos) {
        std::string soPath = libraryPath + ".so";
        handle = dlopen(soPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    }
    if (!handle) {
        const char* err = dlerror();
        std::cerr << "dlopen failed for " << libraryPath << ": " << (err ? err : "Unknown error") << std::endl;
    }
    return handle;
#endif
}

void* PluginLoader::getSymbolAddress(void* handle, const std::string& symbolName) {
    if (!handle) return nullptr;
#if defined(_WIN32) || defined(__WIN32__)
    return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(handle), symbolName.c_str()));
#else
    dlerror();
    void* symbol = dlsym(handle, symbolName.c_str());
    if (dlerror() != nullptr) {
        return nullptr;
    }
    return symbol;
#endif
}

void PluginLoader::unloadLibrary(void* handle) {
    if (!handle) return;
#if defined(_WIN32) || defined(__WIN32__)
    ::FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

} // namespace quasar::scripting
