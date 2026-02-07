#ifndef SMP_INVALIDFIELDVALUE_H
#define SMP_INVALIDFIELDVALUE_H

#include "Smp/AnySimple.h"
#include "Smp/Exception.h"

namespace Smp {
class InvalidFieldValue : public virtual Exception {
public:
  virtual ~InvalidFieldValue() noexcept = default;

  virtual AnySimple GetInvalidFieldValue() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDFIELDVALUE_H
