#include "sim/LibraryLoader.hpp"
#include <iostream>
#include <map>

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace sim {

LibraryLoader &LibraryLoader::GetInstance() {
  static LibraryLoader instance;
  return instance;
}

LibraryLoader::~LibraryLoader() {
  // We could track and close handles here if needed,
  // but often libraries are left loaded until exit.
}

void *LibraryLoader::LoadLibrary(const std::string &libraryPath) {
#if defined(_WIN32) || defined(__WIN32__)
  HMODULE handle = ::LoadLibraryA(libraryPath.c_str());
  if (!handle) {
    throw LibraryException("Failed to load library on Windows: " + libraryPath);
  }
  return static_cast<void *>(handle);
#else
  // Try loading exactly as specified, or append .so
  void *handle = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!handle) {
    std::string soPath = libraryPath + ".so";
    handle = dlopen(soPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
  }

  if (!handle) {
    const char *err = dlerror();
    std::string error = err ? err : "Unknown dlopen error";
    throw LibraryException("Failed to load library " + libraryPath + ": " +
                           error);
  }
  return handle;
#endif
}

bool LibraryLoader::UnloadLibrary(void *handle) {
  if (!handle)
    return false;

#if defined(_WIN32) || defined(__WIN32__)
  return ::FreeLibrary(static_cast<HMODULE>(handle)) != 0;
#else
  return dlclose(handle) == 0;
#endif
}

void *LibraryLoader::GetSymbolAddress(void *handle,
                                      const std::string &symbolName) {
  if (!handle)
    return nullptr;

#if defined(_WIN32) || defined(__WIN32__)
  return reinterpret_cast<void *>(
      ::GetProcAddress(static_cast<HMODULE>(handle), symbolName.c_str()));
#else
  // Clear any existing errors
  dlerror();
  void *symbol = dlsym(handle, symbolName.c_str());
  if (dlerror() != nullptr) {
    return nullptr;
  }
  return symbol;
#endif
}

void *LibraryLoader::GetSymbolAddress(const std::string &libraryPath,
                                      const std::string &symbolName) {
  void *handle = LoadLibrary(libraryPath);
  if (!handle)
    return nullptr;
  return GetSymbolAddress(handle, symbolName);
}

} // namespace sim
