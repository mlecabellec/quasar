#ifndef SMP_IOPERATION_H
#define SMP_IOPERATION_H

#include "Smp/ICollection.h"
#include "Smp/IObject.h"
#include "Smp/IParameter.h"
#include "Smp/IRequest.h"
#include "Smp/InvalidOperationName.h"
#include "Smp/InvalidParameterCount.h"
#include "Smp/InvalidParameterType.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/ViewKind.h"

namespace Smp {
class IOperation : public virtual IObject {
public:
  virtual ~IOperation() noexcept = default;

  virtual const ParameterCollection *GetParameters() const = 0;
  virtual IParameter *GetParameter(String8 name) const = 0;
  virtual IParameter *GetReturnParameter() const = 0;
  virtual ViewKind GetView() const = 0;
  virtual IRequest *CreateRequest() = 0;
  virtual void Invoke(IRequest *request) = 0;
  virtual void DeleteRequest(IRequest *request) = 0;
};

typedef ICollection<IOperation> OperationCollection;
} // namespace Smp

#endif // SMP_IOPERATION_H
