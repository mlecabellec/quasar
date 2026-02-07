#ifndef SMP_IFALLIBLEMODEL_H
#define SMP_IFALLIBLEMODEL_H

#include "Smp/IFailure.h"
#include "Smp/IModel.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IFallibleModel : public virtual IModel {
public:
  virtual ~IFallibleModel() noexcept = default;

  virtual Bool IsFailed() const = 0;
  virtual const FailureCollection *GetFailures() const = 0;
  virtual IFailure *GetFailure(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_IFALLIBLEMODEL_H
