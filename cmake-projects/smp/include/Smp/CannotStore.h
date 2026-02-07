#ifndef SMP_CANNOTSTORE_H
#define SMP_CANNOTSTORE_H

#include "Smp/Exception.h"

namespace Smp {
class CannotStore : public virtual Exception {
public:
  virtual ~CannotStore() noexcept = default;
};
} // namespace Smp

#endif // SMP_CANNOTSTORE_H
