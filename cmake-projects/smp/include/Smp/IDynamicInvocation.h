#ifndef SMP_IDYNAMICINVOCATION_H
#define SMP_IDYNAMICINVOCATION_H

#include "Smp/IComponent.h"
#include "Smp/IOperation.h"
#include "Smp/IProperty.h"
#include "Smp/IRequest.h"
#include "Smp/InvalidOperationName.h"
#include "Smp/InvalidParameterCount.h"
#include "Smp/InvalidParameterType.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IDynamicInvocation : public virtual IComponent {
public:
  virtual ~IDynamicInvocation() noexcept = default;

  virtual IRequest *CreateRequest(String8 operationName) = 0;
  virtual void Invoke(IRequest *request) = 0;
  virtual void DeleteRequest(IRequest *request) = 0;
  virtual const PropertyCollection *GetProperties() const = 0;
  virtual const OperationCollection *GetOperations() const = 0;
};
} // namespace Smp

#endif // SMP_IDYNAMICINVOCATION_H
