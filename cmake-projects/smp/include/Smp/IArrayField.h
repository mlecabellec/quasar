#ifndef SMP_IARRAYFIELD_H
#define SMP_IARRAYFIELD_H

#include "Smp/IField.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IArrayField : public virtual IField {
public:
  virtual ~IArrayField() noexcept = default;

  virtual UInt64 GetSize() const = 0;
  virtual IField *GetItem(UInt64 index) const = 0;
};
} // namespace Smp

#endif // SMP_IARRAYFIELD_H
