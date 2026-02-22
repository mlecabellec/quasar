#include "InverterModel.hpp"
#include <Smp/IFactory.h>
#include <Smp/IPublication.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <iostream>

namespace sample {

// Very basic factory for our model
class InverterFactory : public virtual Smp::IFactory {
public:
  Smp::String8 GetName() const override { return "InverterFactory"; }
  Smp::String8 GetDescription() const override {
    return "Factory for InverterModel";
  }
  Smp::IObject *GetParent() const override { return nullptr; }
  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }
  Smp::Uuid GetUuid() const override {
    return {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};
  }
  Smp::String8 GetTypeName() const override { return "InverterModel"; }

  Smp::IComponent *CreateInstance(Smp::String8 name, Smp::String8 description,
                                  Smp::IComposite *parent) override {
    return new InverterModel(name, description, parent);
  }

  void DeleteInstance(Smp::IComponent *instance) override { delete instance; }
};

static InverterFactory factory;

} // namespace sample

extern "C" {
bool Initialise(Smp::IPublication *receiver,
                Smp::Publication::ITypeRegistry *typeRegistry) {
  std::cout << "[InverterLib] Initialising..." << std::endl;
  // In a real SMP library, we would register our factory here
  return true;
}

bool Finalise() {
  std::cout << "[InverterLib] Finalising..." << std::endl;
  return true;
}

Smp::IFactory *GetFactory(Smp::Uuid uuid) {
  if (uuid == sample::factory.GetUuid()) {
    return &sample::factory;
  }
  return nullptr;
}
}
