#ifndef SMP_ICONTAINER_H
#define SMP_ICONTAINER_H

#include "Smp/CannotDelete.h"
#include "Smp/ContainerFull.h"
#include "Smp/DuplicateName.h"
#include "Smp/ICollection.h"
#include "Smp/IComponent.h"
#include "Smp/IObject.h"
#include "Smp/InvalidObjectType.h"
#include "Smp/NotContained.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class IContainer : public virtual IObject {
public:
  virtual ~IContainer() noexcept = default;

  virtual const ComponentCollection *GetComponents() const = 0;
  virtual IComponent *GetComponent(String8 name) const = 0;
  virtual void AddComponent(IComponent *component) = 0;
  virtual void DeleteComponent(IComponent *component) = 0;
  virtual Int64 GetCount() const = 0;
  virtual Int64 GetUpper() const = 0;
  virtual Int64 GetLower() const = 0;
};

using ContainerCollection = ICollection<IContainer>;
} // namespace Smp

#endif // SMP_ICONTAINER_H
