#pragma once

#include <stdexcept>
#include <string>

namespace sim {

class LibraryLoader {
public:
  static LibraryLoader &GetInstance();

  ~LibraryLoader();

  // Load a dynamic library by name
  void *LoadLibrary(const std::string &libraryPath);

  // Unload a dynamic library
  bool UnloadLibrary(void *handle);

  // Get a symbol from a loaded library
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
