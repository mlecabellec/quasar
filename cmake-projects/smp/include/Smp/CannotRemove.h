#ifndef SMP_CANNOTREMOVE_H
#define SMP_CANNOTREMOVE_H

#include "Smp/Exception.h"
#include "Smp/IComponent.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class CannotRemove : public virtual Exception {
public:
  virtual ~CannotRemove() noexcept = default;

  virtual String8 GetReferenceName() const noexcept = 0;
  virtual const IComponent *GetComponent() const noexcept = 0;
  virtual Int64 GetLowerLimit() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_CANNOTREMOVE_H
