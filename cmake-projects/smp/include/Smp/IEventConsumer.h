#ifndef SMP_IEVENTCONSUMER_H
#define SMP_IEVENTCONSUMER_H

#include "Smp/IComponent.h"
#include "Smp/IEventSink.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IEventConsumer : public virtual IComponent {
public:
  virtual ~IEventConsumer() noexcept = default;

  virtual const EventSinkCollection *GetEventSinks() const = 0;
  virtual IEventSink *GetEventSink(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_IEVENTCONSUMER_H
