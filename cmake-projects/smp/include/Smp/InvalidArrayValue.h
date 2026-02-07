#ifndef SMP_INVALIDARRAYVALUE_H
#define SMP_INVALIDARRAYVALUE_H

#include "Smp/AnySimple.h"
#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidArrayValue : public virtual Exception {
public:
  virtual ~InvalidArrayValue() noexcept = default;

  virtual Int64 GetInvalidValueIndex() const noexcept = 0;
  virtual AnySimple GetInvalidValue() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDARRAYVALUE_H
