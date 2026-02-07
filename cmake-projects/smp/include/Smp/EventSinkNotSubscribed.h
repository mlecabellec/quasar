#ifndef SMP_EVENTSINKNOTSUBSCRIBED_H
#define SMP_EVENTSINKNOTSUBSCRIBED_H

#include "Smp/Exception.h"

namespace Smp {
class IEventSink;
class IEventSource;

class EventSinkNotSubscribed : public virtual Exception {
public:
  virtual ~EventSinkNotSubscribed() noexcept = default;

  virtual const IEventSource *GetEventSource() const noexcept = 0;
  virtual const IEventSink *GetEventSink() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_EVENTSINKNOTSUBSCRIBED_H
