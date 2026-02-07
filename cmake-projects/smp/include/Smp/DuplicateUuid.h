#ifndef SMP_DUPLICATEUUID_H
#define SMP_DUPLICATEUUID_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class DuplicateUuid : public virtual Exception {
public:
  virtual ~DuplicateUuid() noexcept = default;

  virtual String8 GetOldName() const noexcept = 0;
  virtual String8 GetNewName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_DUPLICATEUUID_H
