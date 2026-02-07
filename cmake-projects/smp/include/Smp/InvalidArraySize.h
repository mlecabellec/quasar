#ifndef SMP_INVALIDARRAYSIZE_H
#define SMP_INVALIDARRAYSIZE_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidArraySize : public virtual Exception {
public:
  virtual ~InvalidArraySize() noexcept = default;

  virtual Int64 GetArraySize() const noexcept = 0;
  virtual Int64 GetInvalidSize() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDARRAYSIZE_H
