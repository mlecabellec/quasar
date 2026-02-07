#ifndef SMP_INVALIDFIELDNAME_H
#define SMP_INVALIDFIELDNAME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidFieldName : public virtual Exception {
public:
  virtual ~InvalidFieldName() noexcept = default;

  virtual String8 GetFieldName() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_INVALIDFIELDNAME_H
