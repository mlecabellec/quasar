#ifndef SMP_IFORCIBLEFIELD_H
#define SMP_IFORCIBLEFIELD_H

#include "Smp/AnySimple.h"
#include "Smp/ISimpleField.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IForcibleField : public virtual ISimpleField {
public:
  virtual ~IForcibleField() noexcept = default;

  virtual void Force(AnySimple value) = 0;
  virtual void Unforce() = 0;
  virtual Bool IsForced() = 0;
  virtual void Freeze() = 0;
};
} // namespace Smp

#endif // SMP_IFORCIBLEFIELD_H
