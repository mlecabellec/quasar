#include "DelayedModels.hpp"
#include <Smp/IFactory.h>
#include <Smp/ISimulator.h>

namespace sample {

class DelayedArrayFactory : public virtual Smp::IFactory {
public:
  Smp::String8 GetName() const override { return "DelayedArrayFactory"; }
  Smp::String8 GetDescription() const override { return "Factory for DelayedArrayModel"; }
  Smp::IObject *GetParent() const override { return nullptr; }
  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }
  Smp::Uuid GetUuid() const override { return {0xAAA22222, 0x2222, 0x2222, 0x2222, 0x222222222222}; }
  Smp::String8 GetTypeName() const override { return "DelayedArrayModel"; }
  Smp::IComponent *CreateInstance(Smp::String8 name, Smp::String8 description, Smp::IComposite *parent) override {
    return new DelayedArrayModel(name, description, parent);
  }
  void DeleteInstance(Smp::IComponent *instance) override { if (instance) delete instance; }
};

static DelayedArrayFactory factory;

} // namespace sample

extern "C" {
bool Initialise(Smp::ISimulator *simulator, Smp::Publication::ITypeRegistry *typeRegistry) {
  if (simulator) {
    simulator->RegisterFactory(&sample::factory);
    return true;
  }
  return false;
}
bool Finalise() { return true; }
Smp::IFactory *GetFactory(Smp::Uuid uuid) {
  if (uuid == sample::factory.GetUuid()) return &sample::factory;
  return nullptr;
}
}
