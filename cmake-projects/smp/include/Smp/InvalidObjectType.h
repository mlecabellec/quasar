#ifndef SMP_INVALIDOBJECTTYPE_H
#define SMP_INVALIDOBJECTTYPE_H

#include "Smp/Exception.h"

namespace Smp {
class IObject;

class InvalidObjectType : public virtual Exception {
public:
  virtual ~InvalidObjectType() noexcept = default;

  virtual const IObject *GetInvalidObject() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDOBJECTTYPE_H
