#ifndef SMP_INVALIDEVENTSINK_H
#define SMP_INVALIDEVENTSINK_H

#include "Smp/Exception.h"

namespace Smp {
class IEventSink;
class IEventSource;

class InvalidEventSink : public virtual Exception {
public:
  virtual ~InvalidEventSink() noexcept = default;

  virtual const IEventSource *GetEventSource() const noexcept = 0;
  virtual const IEventSink *GetEventSink() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDEVENTSINK_H
