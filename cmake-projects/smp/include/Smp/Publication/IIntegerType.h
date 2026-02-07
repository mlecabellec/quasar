#ifndef SMP_PUBLICATION_IINTEGERTYPE_H
#define SMP_PUBLICATION_IINTEGERTYPE_H

#include "Smp/Publication/IType.h"

namespace Smp::Publication {

class IIntegerType : public virtual IType {
public:
  virtual ~IIntegerType() noexcept = default;

  virtual Smp::Int64 GetMinimum() const = 0;
  virtual Smp::Int64 GetMaximum() const = 0;
  virtual Smp::String8 GetUnit() const = 0;
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_IINTEGERTYPE_H
