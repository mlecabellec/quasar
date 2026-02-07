#ifndef SMP_INVALIDARRAYINDEX_H
#define SMP_INVALIDARRAYINDEX_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidArrayIndex : public virtual Exception {
public:
  virtual ~InvalidArrayIndex() noexcept = default;

  virtual Int64 GetInvalidIndex() const noexcept = 0;
  virtual Int64 GetArraySize() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDARRAYINDEX_H
