#ifndef SMP_DUPLICATENAME_H
#define SMP_DUPLICATENAME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class DuplicateName : public virtual Exception {
public:
  virtual ~DuplicateName() noexcept = default;

  virtual String8 GetDuplicateName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_DUPLICATENAME_H
