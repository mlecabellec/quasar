#ifndef SMP_REFERENCEFULL_H
#define SMP_REFERENCEFULL_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class ReferenceFull : public virtual Exception {
public:
  virtual ~ReferenceFull() noexcept = default;

  virtual String8 GetReferenceName() const noexcept = 0;
  virtual Int64 GetReferenceSize() const noexcept = 0;
};
} // namespace Smp

#endif // SMP_REFERENCEFULL_H
