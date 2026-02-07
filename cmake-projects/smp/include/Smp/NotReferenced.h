#ifndef SMP_NOTREFERENCED_H
#define SMP_NOTREFERENCED_H

#include "Smp/Exception.h"
#include "Smp/IComponent.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class NotReferenced : public virtual Exception {
public:
  virtual ~NotReferenced() noexcept = default;

  virtual String8 GetReferenceName() const noexcept = 0;
  virtual const IComponent *GetComponent() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_NOTREFERENCED_H
