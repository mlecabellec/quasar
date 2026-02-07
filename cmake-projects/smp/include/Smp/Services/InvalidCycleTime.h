#ifndef SMP_SERVICES_INVALIDCYCLETIME_H
#define SMP_SERVICES_INVALIDCYCLETIME_H

#include "Smp/Exception.h"

namespace Smp {
namespace Services {
class InvalidCycleTime : public virtual Exception {
public:
  virtual ~InvalidCycleTime() noexcept = default;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_INVALIDCYCLETIME_H
