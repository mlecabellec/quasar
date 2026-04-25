#include "InverterModel.hpp"
#include <memory>
#include <Smp/IFactory.h>
#include <Smp/ISimulator.h>
#include <iostream>

namespace sample {

class InverterFactory : public virtual Smp::IFactory {
public:
  Smp::String8 GetName() const override { return "InverterFactory"; }
  Smp::String8 GetDescription() const override { return "Factory for InverterModel"; }
  Smp::IObject *GetParent() const override { return nullptr; }
  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }
  Smp::Uuid GetUuid() const override { return {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678}; }
  Smp::String8 GetTypeName() const override { return "InverterModel"; }
  Smp::IComponent *CreateInstance(Smp::String8 name, Smp::String8 description, Smp::IComposite *parent) override {
    return std::make_unique<InverterModel>(name, description, parent).release();
  }
  void DeleteInstance(Smp::IComponent *instance) override { 
    if (instance) {
        std::unique_ptr<Smp::IComponent> ptr(instance);
    }
  }
};

static InverterFactory factory;

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
