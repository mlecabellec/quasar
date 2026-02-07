#ifndef SMP_LIBRARYNOTFOUND_H
#define SMP_LIBRARYNOTFOUND_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class LibraryNotFound : public virtual Exception {
public:
  virtual ~LibraryNotFound() noexcept = default;

  virtual String8 GetLibraryName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_LIBRARYNOTFOUND_H
