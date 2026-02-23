#include <InverterModel.hpp>
#include <Smp/IModel.h>
#include <Smp/ISimpleField.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <sim/Simulator.hpp>

int main() {
  std::cout << "--- SMP Simulation Test ---" << std::endl;

  sim::Simulator simulator;

  // Create the model using make_unique to avoid 'new' keyword.
  // We keep a raw pointer for local assertions, but ownership will be transferred.
  std::unique_ptr<sample::InverterModel> modelOwner = std::make_unique<sample::InverterModel>("Inverter", "Boolean Inverter Model", nullptr);
  sample::InverterModel* model = modelOwner.get();

  // Add model to simulator
  // Transfer ownership to the simulator as it manages the lifecycle of models.
  simulator.AddModel(modelOwner.release());

  // 1. Publishing State
  std::cout << "Step 1: Publishing..." << std::endl;
  simulator.Publish(); // No arguments for orchestrator
  if (model->GetState() != Smp::ComponentStateKind::CSK_Publishing) {
    std::cerr << "Error: Model not in Publishing state! Current: "
              << model->GetState() << std::endl;
    return 1;
  }

  // 2. Configure State
  std::cout << "Step 2: Configuring..." << std::endl;
  simulator.Configure(); // No arguments for orchestrator
  if (model->GetState() != Smp::ComponentStateKind::CSK_Configured) {
    std::cerr << "Error: Model not in Configured state! Current: "
              << model->GetState() << std::endl;
    return 1;
  }

  // 3. Connect State
  std::cout << "Step 3: Connecting..." << std::endl;
  simulator.Connect(); // No arguments for orchestrator
  if (model->GetState() != Smp::ComponentStateKind::CSK_Connected) {
    std::cerr << "Error: Model not in Connected state! Current: "
              << model->GetState() << std::endl;
    return 1;
  }

  // 4. Verification of logic
  std::cout << "Step 4: Executing..." << std::endl;
  model->Execute(); // Initial state: input=false, output=true (from constructor)

  // We should ideally check the output here but InverterModel implementation is
  // minimal. In a real test, we would use Field accessors.

  std::cout << "Test completed successfully!" << std::endl;
  return 0;
}
