#ifndef SMP_SERVICES_INVALIDEVENTID_H
#define SMP_SERVICES_INVALIDEVENTID_H

#include "Smp/Exception.h"
#include "Smp/Services/EventId.h"

namespace Smp {
namespace Services {
class InvalidEventId : public virtual Exception {
public:
  virtual ~InvalidEventId() noexcept = default;

  virtual EventId GetInvalidEventId() const noexcept = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_INVALIDEVENTID_H
