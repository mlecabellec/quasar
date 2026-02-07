#ifndef SMP_SERVICES_INVALIDEVENTNAME_H
#define SMP_SERVICES_INVALIDEVENTNAME_H

#include "Smp/Exception.h"

namespace Smp {
namespace Services {
class InvalidEventName : public virtual Exception {
public:
  virtual ~InvalidEventName() noexcept = default;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_INVALIDEVENTNAME_H
