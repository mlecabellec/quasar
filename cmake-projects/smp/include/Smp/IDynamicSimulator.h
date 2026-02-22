#ifndef SMP_IDYNAMICSIMULATOR_H
#define SMP_IDYNAMICSIMULATOR_H

#include "Smp/Exception.h"
#include "Smp/IFactory.h"
#include "Smp/ISimulator.h"
#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"

namespace Smp {
class IDynamicSimulator : public virtual ISimulator {
public:
  virtual ~IDynamicSimulator() noexcept = default;

  class DuplicateUuid : public Smp::Exception {
  public:
    String8 newName;
    String8 oldName;

    DuplicateUuid(String8 _newName, String8 _oldName)
        : Smp::Exception("DuplicateUuid", "Duplicate implementation Uuid"),
          newName(_newName), oldName(_oldName) {}
  };

  virtual void RegisterFactory(IFactory *componentFactory) = 0;
  virtual IComponent *CreateInstance(const Uuid implUuid) = 0;
  virtual const IFactory *GetFactory(const Uuid implUuid) const = 0;
  virtual const FactoryCollection *GetFactories(const Uuid specUuid) const = 0;
};
} // namespace Smp

#endif // SMP_IDYNAMICSIMULATOR_H
