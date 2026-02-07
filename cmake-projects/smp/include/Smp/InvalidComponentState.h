#ifndef SMP_INVALIDCOMPONENTSTATE_H
#define SMP_INVALIDCOMPONENTSTATE_H

#include "Smp/ComponentStateKind.h"
#include "Smp/Exception.h"

namespace Smp {
class InvalidComponentState : public virtual Exception {
public:
  virtual ~InvalidComponentState() noexcept = default;

  virtual ComponentStateKind GetInvalidState() const noexcept = 0;
  virtual ComponentStateKind GetExpectedState() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDCOMPONENTSTATE_H
