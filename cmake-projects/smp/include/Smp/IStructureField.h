#ifndef SMP_ISTRUCTUREFIELD_H
#define SMP_ISTRUCTUREFIELD_H

#include "Smp/IField.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IStructureField : public virtual IField {
public:
  virtual ~IStructureField() noexcept = default;

  virtual const FieldCollection *GetFields() const = 0;
  virtual IField *GetField(String8 name) const = 0;
};
} // namespace Smp

#endif // SMP_ISTRUCTUREFIELD_H
