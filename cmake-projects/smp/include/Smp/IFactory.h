#ifndef SMP_IFACTORY_H
#define SMP_IFACTORY_H

#include "Smp/ICollection.h"
#include "Smp/IObject.h"
#include "Smp/InvalidObjectName.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

namespace Smp {
class IComponent;
class IComposite;

class IFactory : public virtual IObject {
public:
  virtual ~IFactory() noexcept = default;

  virtual Uuid GetUuid() const = 0;
  virtual String8 GetTypeName() const = 0;
  virtual IComponent *CreateInstance(String8 name, String8 description,
                                     IComposite *parent) = 0;
  virtual void DeleteInstance(IComponent *instance) = 0;
};

using FactoryCollection = ICollection<IFactory>;
} // namespace Smp

#endif // SMP_IFACTORY_H
