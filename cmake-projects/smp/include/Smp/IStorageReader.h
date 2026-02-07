#ifndef SMP_ISTORAGEREADER_H
#define SMP_ISTORAGEREADER_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IStorageReader {
public:
  virtual ~IStorageReader() noexcept = default;

  virtual void Restore(void *address, UInt64 size) = 0;

  virtual String8 GetStateVectorFileName() const = 0;
  virtual String8 GetStateVectorFilePath() const = 0;
};
} // namespace Smp

#endif // SMP_ISTORAGEREADER_H
