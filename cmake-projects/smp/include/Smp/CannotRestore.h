#ifndef SMP_CANNOTRESTORE_H
#define SMP_CANNOTRESTORE_H

#include "Smp/Exception.h"

namespace Smp {
class CannotRestore : public virtual Exception {
public:
  virtual ~CannotRestore() noexcept = default;
};
} // namespace Smp

#endif // SMP_CANNOTRESTORE_H
