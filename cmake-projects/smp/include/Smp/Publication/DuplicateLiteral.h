#ifndef SMP_PUBLICATION_DUPLICATELITERAL_H
#define SMP_PUBLICATION_DUPLICATELITERAL_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Publication {
class DuplicateLiteral : public virtual Exception {
public:
  virtual ~DuplicateLiteral() noexcept = default;

  virtual String8 GetLiteralName() const noexcept = 0;
  virtual Int32 GetLiteralValue() const noexcept = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_DUPLICATELITERAL_H
