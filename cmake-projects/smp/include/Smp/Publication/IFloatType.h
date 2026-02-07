#ifndef SMP_PUBLICATION_IFLOATTYPE_H
#define SMP_PUBLICATION_IFLOATTYPE_H

#include "Smp/Publication/IType.h"

namespace Smp::Publication {

class IFloatType : public virtual IType {
public:
  virtual ~IFloatType() noexcept = default;

  virtual Smp::Float64 GetMinimum() const = 0;
  virtual Smp::Float64 GetMaximum() const = 0;
  virtual Smp::Bool IsMinInclusive() const = 0;
  virtual Smp::Bool IsMaxInclusive() const = 0;
  virtual Smp::String8 GetUnit() const = 0;
};

} // namespace Smp::Publication

#endif // SMP_PUBLICATION_IFLOATTYPE_H
