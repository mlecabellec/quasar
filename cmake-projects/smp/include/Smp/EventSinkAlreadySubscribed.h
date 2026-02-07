#ifndef SMP_EVENTSINKALREADYSUBSCRIBED_H
#define SMP_EVENTSINKALREADYSUBSCRIBED_H

#include "Smp/Exception.h"

namespace Smp {
class IEventSink;
class IEventSource;

class EventSinkAlreadySubscribed : public virtual Exception {
public:
  virtual ~EventSinkAlreadySubscribed() noexcept = default;

  virtual const IEventSink *GetEventSink() const noexcept = 0;
  virtual const IEventSource *GetEventSource() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_EVENTSINKALREADYSUBSCRIBED_H
