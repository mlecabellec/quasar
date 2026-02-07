#ifndef SMP_IPROPERTY_H
#define SMP_IPROPERTY_H

#include "Smp/AccessKind.h"
#include "Smp/AnySimple.h"
#include "Smp/ICollection.h"
#include "Smp/IObject.h"
#include "Smp/Publication/IType.h"
#include "Smp/ViewKind.h"

namespace Smp {
class IProperty : public virtual IObject {
public:
  virtual ~IProperty() noexcept = default;

  virtual Publication::IType *GetType() const = 0;
  virtual AccessKind GetAccess() const = 0;
  virtual ViewKind GetView() const = 0;
  virtual AnySimple GetValue() const = 0;
  virtual void SetValue(AnySimple value) = 0;
};

typedef ICollection<IProperty> PropertyCollection;
} // namespace Smp

#endif // SMP_IPROPERTY_H
