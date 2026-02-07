#ifndef SMP_PUBLICATION_ISTRINGTYPE_H
#define SMP_PUBLICATION_ISTRINGTYPE_H

#include "Smp/Publication/IType.h"

namespace Smp::Publication {

class IStringType : public virtual IType {
public:
  virtual ~IStringType() noexcept = default;

  virtual Smp::Int64 GetLength() const = 0;
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_ISTRINGTYPE_H
