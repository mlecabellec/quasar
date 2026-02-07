#ifndef SMP_PUBLICATION_IENUMERATIONTYPE_H
#define SMP_PUBLICATION_IENUMERATIONTYPE_H

#include "Smp/DuplicateName.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Publication/DuplicateLiteral.h"
#include "Smp/Publication/IType.h"

namespace Smp {
namespace Publication {
class IEnumerationType : public virtual IType {
public:
  virtual ~IEnumerationType() noexcept = default;

  virtual void AddLiteral(String8 name, String8 description, Int32 value) = 0;
};
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_IENUMERATIONTYPE_H
