#ifndef SMP_IREQUEST_H
#define SMP_IREQUEST_H

#include "Smp/AnySimple.h"
#include "Smp/InvalidAnyType.h"
#include "Smp/InvalidParameterIndex.h"
#include "Smp/InvalidParameterValue.h"
#include "Smp/InvalidReturnValue.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/VoidOperation.h"

namespace Smp {
class IRequest {
public:
  virtual ~IRequest() noexcept = default;

  virtual String8 GetOperationName() const = 0;
  virtual Int32 GetParameterCount() const = 0;
  virtual Int32 GetParameterIndex(String8 parameterName) const = 0;
  virtual void SetParameterValue(Int32 index, AnySimple value) = 0;
  virtual AnySimple GetParameterValue(Int32 index) const = 0;
  virtual void SetReturnValue(AnySimple value) = 0;
  virtual AnySimple GetReturnValue() const = 0;
};
} // namespace Smp

#endif // SMP_IREQUEST_H
