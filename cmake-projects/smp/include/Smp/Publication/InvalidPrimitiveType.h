#ifndef SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H
#define SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Publication {
class InvalidPrimitiveType : public virtual Exception {
public:
  virtual ~InvalidPrimitiveType() noexcept = default;

  virtual String8 GetTypeName() const noexcept = 0;
  virtual PrimitiveTypeKind GetType() const noexcept = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_INVALIDPRIMITIVETYPE_H
