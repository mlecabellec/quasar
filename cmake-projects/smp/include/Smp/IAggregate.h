#ifndef SMP_IAGGREGATE_H
#define SMP_IAGGREGATE_H

#include "Smp/IComponent.h"
#include "Smp/IReference.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IAggregate : public virtual IComponent {
public:
  virtual ~IAggregate() noexcept = default;

  virtual const ReferenceCollection *GetReferences() const = 0;
  virtual IReference *GetReference(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_IAGGREGATE_H
