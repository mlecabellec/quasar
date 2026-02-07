#ifndef SMP_ISIMPLEARRAYFIELD_H
#define SMP_ISIMPLEARRAYFIELD_H

#include "Smp/AnySimple.h"
#include "Smp/AnySimpleArray.h"
#include "Smp/IArrayField.h"
#include "Smp/InvalidArrayIndex.h"
#include "Smp/InvalidArraySize.h"
#include "Smp/InvalidArrayValue.h"
#include "Smp/InvalidFieldValue.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class ISimpleArrayField : public virtual IArrayField {
public:
  virtual ~ISimpleArrayField() noexcept = default;

  virtual UInt64 GetSize() const = 0;
  virtual AnySimple GetValue(UInt64 index) const = 0;
  virtual void SetValue(UInt64 index, AnySimple value) = 0;
  virtual void GetValues(UInt64 length, AnySimpleArray values) const = 0;
  virtual void SetValues(UInt64 length, AnySimpleArray values) = 0;
};
} // namespace Smp

#endif // SMP_ISIMPLEARRAYFIELD_H
