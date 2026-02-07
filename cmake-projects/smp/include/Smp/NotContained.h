#ifndef SMP_NOTCONTAINED_H
#define SMP_NOTCONTAINED_H

#include "Smp/Exception.h"
#include "Smp/IComponent.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class NotContained : public virtual Exception {
public:
  virtual ~NotContained() noexcept = default;

  virtual String8 GetContainerName() const noexcept = 0;
  virtual const IComponent *GetComponent() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_NOTCONTAINED_H
