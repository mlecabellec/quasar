#ifndef SMP_INVALIDOPERATIONNAME_H
#define SMP_INVALIDOPERATIONNAME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidOperationName : public virtual Exception {
public:
  virtual ~InvalidOperationName() noexcept = default;

  virtual String8 GetOperationName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDOPERATIONNAME_H
