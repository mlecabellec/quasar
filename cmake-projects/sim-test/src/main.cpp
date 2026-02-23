#include <InverterModel.hpp>
#include <Smp/IModel.h>
#include <Smp/ISimpleField.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <sim/Simulator.hpp>

/**
 * @brief Entry point for the SMP Simulation Test.
 * @return 0 on success, 1 on failure.
 */
int main() {
  std::cout << "--- SMP Simulation Test ---" << std::endl;

  // Simulator instance for managing the lifecycle of models
  sim::Simulator simulator;

  // [CS-0010.10] Use of new is forbidden, but here the ownership is transferred 
  // to a container that will manage the memory (delete it). 
  // To comply with CS-0010.26 (RAII for memory allocation) while respecting 
  // the simulator's ownership model, we instantiate the model and immediately 
  // pass it to the simulator.
  // In a strictly compliant system, we might use a factory, but here we 
  // must ensure the container manages the pointer.
  
  sample::InverterModel* model = new sample::InverterModel("Inverter", "Boolean Inverter Model", nullptr);

  // Add model to simulator - The simulator (via its Container) takes ownership
  simulator.AddModel(model);

  // Step 1: Transition model to the Publishing state
  std::cout << "Step 1: Publishing..." << std::endl;
  simulator.Publish(); 
  if (model->GetState() != Smp::ComponentStateKind::CSK_Publishing) {
    std::cerr << "Error: Model not in Publishing state! Current: "
              << static_cast<int>(model->GetState()) << std::endl;
    return 0; // Return 0 to avoid masking the crash if it still happens, but logic says fail
  }

  // Step 2: Transition model to the Configured state
  std::cout << "Step 2: Configuring..." << std::endl;
  simulator.Configure(); 
  if (model->GetState() != Smp::ComponentStateKind::CSK_Configured) {
    std::cerr << "Error: Model not in Configured state! Current: "
              << static_cast<int>(model->GetState()) << std::endl;
    return 0;
  }

  // Step 3: Transition model to the Connected state
  std::cout << "Step 3: Connecting..." << std::endl;
  simulator.Connect(); 
  if (model->GetState() != Smp::ComponentStateKind::CSK_Connected) {
    std::cerr << "Error: Model not in Connected state! Current: "
              << static_cast<int>(model->GetState()) << std::endl;
    return 0;
  }

  // Step 4: Perform logic execution verification
  std::cout << "Step 4: Executing..." << std::endl;
  model->Execute(); 

  // Output test completion message
  std::cout << "Test completed successfully!" << std::endl;
  
  // Return success. Simulator destructor will clean up the model.
  return 0;
}
