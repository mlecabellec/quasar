#ifndef SMP_IEVENTSOURCE_H
#define SMP_IEVENTSOURCE_H

#include "Smp/EventSinkAlreadySubscribed.h"
#include "Smp/EventSinkNotSubscribed.h"
#include "Smp/ICollection.h"
#include "Smp/IEventSink.h"
#include "Smp/IObject.h"
#include "Smp/InvalidEventSink.h"

namespace Smp {
class IEventSource : public virtual IObject {
public:
  virtual ~IEventSource() noexcept = default;

  virtual void Subscribe(IEventSink *eventSink) = 0;
  virtual void Unsubscribe(IEventSink *eventSink) = 0;
};

typedef ICollection<IEventSource> EventSourceCollection;
} // namespace Smp

#endif // SMP_IEVENTSOURCE_H
