#ifndef SMP_PUBLICATION_ISTRUCTURETYPE_H
#define SMP_PUBLICATION_ISTRUCTURETYPE_H

#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/IType.h"
#include "Smp/Uuid.h"
#include "Smp/ViewKind.h"

namespace Smp {
namespace Publication {
class IStructureType : public virtual IType {
public:
  virtual ~IStructureType() noexcept = default;

  virtual void AddField(String8 name, String8 description, Uuid uuid,
                        Int64 offset, ViewKind view = ViewKind::VK_All,
                        Bool state = true, Bool input = false,
                        Bool output = false) = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_ISTRUCTURETYPE_H
