#include "InverterModel.hpp"
#include <Smp/IFactory.h>
#include <Smp/IPublication.h>
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <iostream>

namespace sample {

/**
 * @brief Factory for the InverterModel.
 * @details Required for dynamic instantiation by the simulator.
 */
class InverterFactory : public virtual Smp::IFactory {
public:
  Smp::String8 GetName() const override { return "InverterFactory"; }
  Smp::String8 GetDescription() const override {
    return "Factory for InverterModel";
  }
  Smp::IObject *GetParent() const override { return nullptr; }
  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }
  
  /** @brief UUID of the InverterModel. */
  Smp::Uuid GetUuid() const override {
    return {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};
  }
  
  Smp::String8 GetTypeName() const override { return "InverterModel"; }

  Smp::IComponent *CreateInstance(Smp::String8 name, Smp::String8 description,
                                  Smp::IComposite *parent) override {
    // Note: The Simulator/Container takes ownership and will call delete via its own mechanisms
    return new InverterModel(name, description, parent);
  }

  void DeleteInstance(Smp::IComponent *instance) override { 
    if (instance) {
      delete instance; 
    }
  }
};

/** @brief Static instance of the factory. */
static InverterFactory factory;

} // namespace sample

extern "C" {

/**
 * @brief Library entry point called by the simulator after loading.
 * @param simulator Pointer to the simulator service.
 * @param typeRegistry Pointer to the type registry service.
 * @return True on success, false otherwise.
 */
bool Initialise(Smp::ISimulator *simulator,
                Smp::Publication::ITypeRegistry *typeRegistry) {
  std::cout << "[InverterLib] Initialising and registering factory..." << std::endl;
  
  if (simulator) {
    // Register the factory so the simulator can create instances by UUID
    simulator->RegisterFactory(&sample::factory);
    return true;
  }
  
  return false;
}

/**
 * @brief Library entry point called before unloading.
 * @return True.
 */
bool Finalise() {
  std::cout << "[InverterLib] Finalising..." << std::endl;
  return true;
}

/**
 * @brief Get a factory by UUID (alternative mechanism).
 */
Smp::IFactory *GetFactory(Smp::Uuid uuid) {
  if (uuid == sample::factory.GetUuid()) {
    return &sample::factory;
  }
  return nullptr;
}
}
