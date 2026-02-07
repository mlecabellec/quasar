#ifndef SMP_IFIELD_H
#define SMP_IFIELD_H

#include "Smp/ICollection.h"
#include "Smp/IPersist.h"
#include "Smp/Publication/IType.h"
#include "Smp/ViewKind.h"

namespace Smp {
class IField : public virtual IPersist {
public:
  virtual ~IField() noexcept = default;

  virtual ViewKind GetView() const = 0;
  virtual Bool IsState() const = 0;
  virtual Bool IsInput() const = 0;
  virtual Bool IsOutput() const = 0;
  virtual const Publication::IType *GetType() const = 0;
};

using FieldCollection = ICollection<IField>;
} // namespace Smp

#endif // SMP_IFIELD_H
