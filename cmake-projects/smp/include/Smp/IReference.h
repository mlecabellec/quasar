#ifndef SMP_IREFERENCE_H
#define SMP_IREFERENCE_H

#include "Smp/CannotRemove.h"
#include "Smp/ICollection.h"
#include "Smp/IComponent.h"
#include "Smp/IObject.h"
#include "Smp/InvalidObjectType.h"
#include "Smp/NotReferenced.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/ReferenceFull.h"

namespace Smp {
class IReference : public virtual IObject {
public:
  virtual ~IReference() noexcept = default;

  virtual const ComponentCollection *GetComponents() const = 0;
  virtual IComponent *GetComponent(String8 name) const = 0;
  virtual void AddComponent(IComponent *component) = 0;
  virtual void RemoveComponent(IComponent *component) = 0;
  virtual Int64 GetCount() const = 0;
  virtual Int64 GetUpper() const = 0;
  virtual Int64 GetLower() const = 0;
};

typedef ICollection<IReference> ReferenceCollection;
} // namespace Smp

#endif // SMP_IREFERENCE_H
