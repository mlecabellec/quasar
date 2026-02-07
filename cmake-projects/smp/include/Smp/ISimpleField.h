#ifndef SMP_ISIMPLEFIELD_H
#define SMP_ISIMPLEFIELD_H

#include "Smp/AnySimple.h"
#include "Smp/IField.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class ISimpleField : public virtual IField {
public:
  virtual ~ISimpleField() noexcept = default;

  virtual PrimitiveTypeKind GetPrimitiveTypeKind() const = 0;
  virtual AnySimple GetValue() const = 0;
  virtual void SetValue(AnySimple value) = 0;
};
} // namespace Smp

#endif // SMP_ISIMPLEFIELD_H
