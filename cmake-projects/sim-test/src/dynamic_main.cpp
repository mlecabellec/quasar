#include <Smp/IComponent.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <Smp/ISimpleField.h>
#include <Smp/IEntryPoint.h>
#include <iostream>
#include <sim/Simulator.hpp>

/**
 * @brief Test executable for dynamic loading of SMP models.
 * @details This test proves that a model can be loaded from a shared library
 * and instantiated without compile-time linking to the model's library.
 */
int main() {
  std::cout << "--- SMP Dynamic Loading Test ---" << std::endl;

  // Instantiate the simulator
  sim::Simulator simulator;

  // Path to the shared library. In a build tree, it's usually in lib/
  const char* libraryPath = "lib/libsample-models.so";
  
  std::cout << "Loading library: " << libraryPath << std::endl;
  try {
    simulator.LoadLibrary(const_cast<char*>(libraryPath));
  } catch (const std::exception& e) {
    std::cerr << "Failed to load library: " << e.what() << std::endl;
    // Fallback for different build structures
    libraryPath = "libsample-models.so";
    try {
        simulator.LoadLibrary(const_cast<char*>(libraryPath));
    } catch (const std::exception& e2) {
        std::cerr << "Absolute fallback failed: " << e2.what() << std::endl;
        return 1;
    }
  }

  // UUID of the InverterModel as defined in Library.cpp
  Smp::Uuid inverterUuid = {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};

  std::cout << "Creating instance of InverterModel via factory..." << std::endl;
  Smp::IComponent* component = simulator.CreateInstance(inverterUuid, "DynamicInverter", "Created via LoadLibrary", nullptr);

  if (!component) {
    std::cerr << "Error: Failed to create instance via factory!" << std::endl;
    return 1;
  }

  std::cout << "Successfully created component: " << component->GetName() << std::endl;

  // Add the model to the simulator (this also transfers ownership to the Models container)
  Smp::IModel* model = dynamic_cast<Smp::IModel*>(component);
  if (model) {
      simulator.AddModel(model);
  } else {
      std::cerr << "Error: component is not an IModel!" << std::endl;
      return 1;
  }

  // Perform state transitions via standard IComponent interface
  std::cout << "Step 1: Publishing..." << std::endl;
  simulator.Publish();
  if (component->GetState() != Smp::ComponentStateKind::CSK_Publishing) {
      std::cerr << "State error after Publish: " << (int)component->GetState() << std::endl;
      return 1;
  }
  
  std::cout << "Step 2: Configuring..." << std::endl;
  simulator.Configure();
  if (component->GetState() != Smp::ComponentStateKind::CSK_Configured) {
      std::cerr << "State error after Configure: " << (int)component->GetState() << std::endl;
      return 1;
  }
  
  std::cout << "Step 3: Connecting..." << std::endl;
  simulator.Connect();
  if (component->GetState() != Smp::ComponentStateKind::CSK_Connected) {
      std::cerr << "State error after Connect: " << (int)component->GetState() << std::endl;
      return 1;
  }

  std::cout << "Step 4: Connecting Input and Output and testing behavior..." << std::endl;
  
  // Use GetField to find the fields by name
  Smp::ISimpleField* inputField = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input"));
  Smp::ISimpleField* outputField = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output"));

  if (!inputField || !outputField) {
      std::cerr << "Error: Could not find Input or Output fields via SMP interface!" << std::endl;
      return 1;
  }

  // Find the entry point for execution
  Smp::IEntryPoint* executeEntryPoint = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));
  if (!executeEntryPoint) {
      std::cerr << "Error: Could not find Execute entry point via GetChild!" << std::endl;
      return 1;
  }

  // Test 1: Set Input to false, check Output is true after Execute
  inputField->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, false));
  executeEntryPoint->Execute();
  if (!(bool)outputField->GetValue()) {
      std::cerr << "Dynamic test failed: expected true output for false input" << std::endl;
      return 1;
  }
  std::cout << "  Dynamic Test 1 (false -> true) passed." << std::endl;

  // Test 2: Feedback connection (Output -> Input)
  inputField->SetValue(outputField->GetValue()); // Set Input to true (as Output was true)
  executeEntryPoint->Execute();
  if ((bool)outputField->GetValue()) {
      std::cerr << "Dynamic test failed: expected false output for true input (feedback)" << std::endl;
      return 1;
  }
  std::cout << "  Dynamic Test 2 (true -> false via feedback) passed." << std::endl;

  std::cout << "Dynamic Loading Test completed successfully!" << std::endl;
  return 0;
}
