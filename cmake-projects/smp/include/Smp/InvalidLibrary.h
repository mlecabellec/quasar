#ifndef SMP_INVALIDLIBRARY_H
#define SMP_INVALIDLIBRARY_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidLibrary : public virtual Exception {
public:
  virtual ~InvalidLibrary() noexcept = default;

  virtual String8 GetLibraryName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDLIBRARY_H
