#ifndef SMP_PUBLICATION_ICLASSTYPE_H
#define SMP_PUBLICATION_ICLASSTYPE_H

#include "Smp/Publication/IStructureType.h"

namespace Smp {
namespace Publication {
class IClassType : public virtual IStructureType {
public:
  virtual ~IClassType() noexcept = default;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_ICLASSTYPE_H
