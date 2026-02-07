#ifndef SMP_PUBLICATION_IARRAYTYPE_H
#define SMP_PUBLICATION_IARRAYTYPE_H

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IType.h"

namespace Smp {
namespace Publication {
class IArrayType : public virtual IType {
public:
  virtual ~IArrayType() noexcept = default;

  virtual UInt64 GetSize() const = 0;
  virtual const IType *GetItemType() const = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_IARRAYTYPE_H
