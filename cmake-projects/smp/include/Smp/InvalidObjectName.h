#ifndef SMP_INVALIDOBJECTNAME_H
#define SMP_INVALIDOBJECTNAME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidObjectName : public virtual Exception {
public:
  virtual ~InvalidObjectName() noexcept = default;

  virtual String8 GetInvalidName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDOBJECTNAME_H
