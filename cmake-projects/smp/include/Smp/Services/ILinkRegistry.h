#ifndef SMP_SERVICES_ILINKREGISTRY_H
#define SMP_SERVICES_ILINKREGISTRY_H

#include "Smp/IComponent.h"
#include "Smp/IService.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Services {
class ILinkRegistry : public virtual IService {
public:
  static constexpr Char8 SMP_LinkRegistry[] = "LinkRegistry";

  virtual ~ILinkRegistry() noexcept = default;

  virtual void AddLink(IComponent *source, const IComponent *target) = 0;
  virtual UInt32 GetLinkCount(const IComponent *source,
                              const IComponent *target) const = 0;
  virtual Bool RemoveLink(IComponent *source, const IComponent *target) = 0;
  virtual const ComponentCollection *
  GetLinkSources(const IComponent *target) const = 0;
  virtual Bool CanRemove(const IComponent *target) = 0;
  virtual void RemoveLinks(const IComponent *target) = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_ILINKREGISTRY_H
