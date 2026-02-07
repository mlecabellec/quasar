#ifndef SMP_ILINKINGCOMPONENT_H
#define SMP_ILINKINGCOMPONENT_H

#include "Smp/IComponent.h"

namespace Smp {
class ILinkingComponent : public virtual IComponent {
public:
  virtual ~ILinkingComponent() noexcept = default;

  virtual void RemoveLinks(const IComponent *target) = 0;
};
} // namespace Smp

#endif // SMP_ILINKINGCOMPONENT_H
