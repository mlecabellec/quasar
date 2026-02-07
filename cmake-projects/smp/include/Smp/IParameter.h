#ifndef SMP_IPARAMETER_H
#define SMP_IPARAMETER_H

#include "Smp/ICollection.h"
#include "Smp/IObject.h"
#include "Smp/Publication/IType.h"
#include "Smp/Publication/ParameterDirectionKind.h"

namespace Smp {
class IParameter : public virtual IObject {
public:
  virtual ~IParameter() noexcept = default;

  virtual Publication::IType *GetType() const = 0;
  virtual Publication::ParameterDirectionKind GetDirection() const = 0;
};

typedef ICollection<IParameter> ParameterCollection;
} // namespace Smp

#endif // SMP_IPARAMETER_H
