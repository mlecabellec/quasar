#ifndef SMP_INVALIDPARAMETERTYPE_H
#define SMP_INVALIDPARAMETERTYPE_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidParameterType : public virtual Exception {
public:
  virtual ~InvalidParameterType() noexcept = default;

  virtual String8 GetOperationName() const noexcept = 0;
  virtual String8 GetParameterName() const noexcept = 0;
  virtual PrimitiveTypeKind GetInvalidType() const noexcept = 0;
  virtual PrimitiveTypeKind GetExpectedType() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDPARAMETERTYPE_H
