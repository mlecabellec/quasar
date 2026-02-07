#ifndef SMP_PUBLICATION_ITYPE_H
#define SMP_PUBLICATION_ITYPE_H

#include "Smp/IObject.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"

namespace Smp {
class IPublication;

namespace Publication {
class IType : public virtual IObject {
public:
  virtual ~IType() noexcept = default;

  virtual PrimitiveTypeKind GetPrimitiveTypeKind() const = 0;
  virtual Uuid GetUuid() const = 0;
  virtual void Publish(IPublication *receiver, String8 name,
                       String8 description, void *address,
                       ViewKind view = ViewKind::VK_All, Bool state = true,
                       Bool input = false, Bool output = false) = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_ITYPE_H
