#ifndef SMP_IEVENTPROVIDER_H
#define SMP_IEVENTPROVIDER_H

#include "Smp/IComponent.h"
#include "Smp/IEventSource.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IEventProvider : public virtual IComponent {
public:
  virtual ~IEventProvider() noexcept = default;

  virtual const EventSourceCollection *GetEventSources() const = 0;
  virtual IEventSource *GetEventSource(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_IEVENTPROVIDER_H
