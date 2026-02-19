#ifndef SMP_LIBRARYLOADINGFLAG_H
#define SMP_LIBRARYLOADINGFLAG_H

#include "Smp/PrimitiveTypes.h"
#include <iosfwd>

namespace Smp {
/// This flag defines whether the symbols in a library shall be loaded as
/// local symbols, global symbols, or using the default setting of the
/// simulation infrastructure.
enum class LibraryLoadingFlag : Int32 {
  /// Load the library using the default setting of the simulation
  /// environment.
  LLF_Auto,

  /// Load the library with global flag, making the symbols available
  /// globally.
  LLF_Global,

  /// Load the library with local flag, making the symbols only available
  /// locally within the library.
  LLF_Local
};

/// Stream operator for LibraryLoadingFlag to be able to print an enum value.
std::ostream &operator<<(std::ostream &os, const LibraryLoadingFlag &obj);
} // namespace Smp

#endif // SMP_LIBRARYLOADINGFLAG_H
