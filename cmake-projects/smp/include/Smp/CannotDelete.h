#ifndef SMP_CANNOTDELETE_H
#define SMP_CANNOTDELETE_H

#include "Smp/Exception.h"
#include "Smp/IComponent.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class CannotDelete : public virtual Exception {
public:
  virtual ~CannotDelete() noexcept = default;

  virtual String8 GetContainerName() const noexcept = 0;
  virtual const IComponent *GetComponent() const noexcept = 0;
  virtual Int64 GetLowerLimit() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_CANNOTDELETE_H
