#pragma once

#include <stdexcept>
#include <string>

namespace sim {

/**
 * @brief Helper for dynamic library loading.
 * @details Contributes to [FE-0080.7] (Package to Library Mapping) and [FE-0070.7.31] (ISimulator::LoadLibrary).
 */
class LibraryLoader {
public:
  static LibraryLoader &GetInstance();

  ~LibraryLoader();

  /// [FE-0080.7.5/6] Load a dynamic library (.so or .dll).
  void *LoadLibrary(const std::string &libraryPath);

  // Unload a dynamic library
  bool UnloadLibrary(void *handle);

  /// [FE-0080.7.2/3] Get a symbol (e.g., Initialise, Finalise) from a loaded library.
  void *GetSymbolAddress(void *handle, const std::string &symbolName);

  // Also provide a helper that finds the symbol across all loaded libraries if
  // possible
  void *GetSymbolAddress(const std::string &libraryPath,
                         const std::string &symbolName);

private:
  LibraryLoader() = default;
};

class LibraryException : public std::runtime_error {
public:
  explicit LibraryException(const std::string &message)
      : std::runtime_error(message) {}
};

} // namespace sim
