#ifndef SMP_ISTORAGEWRITER_H
#define SMP_ISTORAGEWRITER_H

#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IStorageWriter {
public:
  virtual ~IStorageWriter() noexcept = default;

  virtual void Store(void *address, UInt64 size) = 0;

  virtual String8 GetStateVectorFileName() const = 0;
  virtual String8 GetStateVectorFilePath() const = 0;
};
} // namespace Smp

#endif // SMP_ISTORAGEWRITER_H
