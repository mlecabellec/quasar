#ifndef SMP_INVALIDFIELDTYPE_H
#define SMP_INVALIDFIELDTYPE_H

#include "Smp/Exception.h"

namespace Smp {
class InvalidFieldType : public virtual Exception {
public:
  virtual ~InvalidFieldType() noexcept = default;
};
} // namespace Smp

#endif // SMP_INVALIDFIELDTYPE_H
