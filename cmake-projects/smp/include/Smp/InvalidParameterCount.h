#ifndef SMP_INVALIDPARAMETERCOUNT_H
#define SMP_INVALIDPARAMETERCOUNT_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidParameterCount : public virtual Exception {
public:
  virtual ~InvalidParameterCount() noexcept = default;

  virtual String8 GetOperationName() const noexcept = 0;
  virtual Int32 GetOperationParameters() const noexcept = 0;
  virtual Int32 GetRequestParameters() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDPARAMETERCOUNT_H
