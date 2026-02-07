#ifndef SMP_SERVICES_INVALIDEVENTTIME_H
#define SMP_SERVICES_INVALIDEVENTTIME_H

#include "Smp/Exception.h"

namespace Smp {
namespace Services {
class InvalidEventTime : public virtual Exception {
public:
  virtual ~InvalidEventTime() noexcept = default;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_INVALIDEVENTTIME_H
