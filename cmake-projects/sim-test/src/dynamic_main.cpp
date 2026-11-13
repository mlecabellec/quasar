/**
 * @file dynamic_main.cpp
 * @brief Dynamic SMP model loading test verifying factory instantiation and execution without link-time dependency.
 *
 * @details Loads `libsample-inverter.so` using a bounded three-path fallback strategy, instantiates
 * the inverter component by UUID, drives SMP lifecycle transitions, resolves fields and entry points
 * dynamically, and validates boolean inversion with feedback.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Enforces full Doxygen contracts, traceability citations, control-flow comments,
 * and CS-0010.44 density rules without altering executable test statements.
 */

// SMP standard interfaces for components, models, simulator, fields, and entry points.
#include <Smp/IComponent.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <Smp/ISimpleField.h>
#include <Smp/IEntryPoint.h>
// Console I/O and Quasar simulator runtime implementation.
#include <iostream>
#include <sim/Simulator.hpp>

/**
 * @brief Test executable for dynamic loading of SMP models.
 *
 * @details This test proves that a model can be loaded from a shared library
 * and instantiated without compile-time linking to the model's library, verifying
 * factory creation, `IModel` registration, SMP state transitions, field lookup,
 * and `Execute` entry-point invocation.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.15, CS-0010.20, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Documents bounded three-attempt library loading containment, null/type guards,
 * return semantics, and every conditional/exception branch.
 *
 * @post All simulator resources and dynamically loaded models are released on exit.
 * @return int
 *   Justification: Reports test execution outcome to CTest or shell caller.
 *   Usage: Returned directly from `main` to indicate pass (0) or failure (1).
 *   Consumed: Scalar value copied to C runtime exit handler.
 *   Prerequisites: None (process entry point).
 *   Limitations: Does not explicitly unload shared object handles before simulator destruction.
 *   Lifecycle: Temporary integer exit status valid at process termination.
 *   Modification / Deletion: Immutable prvalue returned to OS; no caller cleanup needed.
 *   Specific Values: 0 (all dynamic loading and inversion checks passed); 1 (library load, state, lookup, or behavior failed).
 *   Error Behavior: Logs failure details to `std::cerr` and returns 1 on handled load or assertion errors.
 * @retval 0 Dynamic loading, lifecycle, and inversion checks succeeded.
 * @retval 1 Shared library load failed or component/state/field/behavior check failed.
 * @throws std::exception [Resource / Fatal] Uncaught simulator exceptions outside the library-load blocks propagate to terminate.
 * @error
 *   - Hardware / Bus / I/O (shared library missing at candidate paths): Caught across three nested `try`/`catch` blocks; logs each attempt and returns 1 if all fail.
 *   - Validation / Contract (null factory result, non-`IModel` component, state mismatch, missing field/entry point, or wrong inversion output): Logs diagnostic to `std::cerr` and returns 1.
 * @containment
 *   Approach: Bounded three-level `try`/`catch (const std::exception&)` isolates `dlopen` path
 *   differences across build directories, followed by explicit null and `dynamic_cast` guards.
 *   Limitations: Operations after library loading (such as `Publish`/`Execute`) are not wrapped
 *   in a top-level `try`/`catch` and rely on simulator destruction for cleanup if they throw.
 */
int main() {
  std::cout << "--- SMP Dynamic Loading Test ---" << std::endl;

  // Instantiate the simulator
  sim::Simulator simulator;

  // Path to the shared library. In a build tree, it's usually in lib/
  const char* libraryPath = "../../lib/libsample-inverter.so";
  
  std::cout << "Loading library: " << libraryPath << std::endl;
  // Attempt primary build-tree shared library path; cascade through fallbacks on failure.
  try {
    simulator.LoadLibrary(const_cast<char*>(libraryPath));
  } catch (const std::exception& e) {
    // Handle primary path load failure by falling back to local lib/ directory.
    std::cerr << "Failed to load library: " << e.what() << std::endl;
    // Fallback for different build structures
    libraryPath = "lib/libsample-inverter.so";
    // Attempt secondary relative path before trying current working directory.
    try {
        simulator.LoadLibrary(const_cast<char*>(libraryPath));
    } catch (const std::exception& e2) {
        // Handle secondary load failure with final current-directory probe.
        std::cerr << "Fallback failed: " << e2.what() << std::endl;
        libraryPath = "libsample-inverter.so";
        // Attempt final library path; abort test with exit code 1 if unavailable.
        try {
            simulator.LoadLibrary(const_cast<char*>(libraryPath));
        } catch (const std::exception& e3) {
            // All bounded library probe attempts failed; report diagnostic and fail cleanly.
            std::cerr << "Absolute fallback failed: " << e3.what() << std::endl;
            return 1;
        }
    }
  }

  // UUID of the InverterModel as defined in Library.cpp
  Smp::Uuid inverterUuid = {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};

  std::cout << "Creating instance of InverterModel via factory..." << std::endl;
  Smp::IComponent* component = simulator.CreateInstance(inverterUuid, "DynamicInverter", "Created via LoadLibrary", nullptr);

  // Verify factory returned a valid component instance before querying properties [CS-0010.15].
  if (!component) {
    std::cerr << "Error: Failed to create instance via factory!" << std::endl;
    return 1;
  }

  std::cout << "Successfully created component: " << component->GetName() << std::endl;

  // Add the model to the simulator (this also transfers ownership to the Models container)
  Smp::IModel* model = dynamic_cast<Smp::IModel*>(component);
  // Register component as IModel when dynamic_cast succeeds; otherwise abort with error.
  if (model) {
      simulator.AddModel(model);
  } else {
      // Component lacked Smp::IModel interface required by simulator model container.
      std::cerr << "Error: component is not an IModel!" << std::endl;
      return 1;
  }

  // Perform state transitions via standard IComponent interface
  std::cout << "Step 1: Publishing..." << std::endl;
  simulator.Publish();
  // Confirm component reached CSK_Publishing state after simulator Publish.
  if (component->GetState() != Smp::ComponentStateKind::CSK_Publishing) {
      std::cerr << "State error after Publish: " << (int)component->GetState() << std::endl;
      return 1;
  }
  
  std::cout << "Step 2: Configuring..." << std::endl;
  simulator.Configure();
  // Confirm component reached CSK_Configured state after simulator Configure.
  if (component->GetState() != Smp::ComponentStateKind::CSK_Configured) {
      std::cerr << "State error after Configure: " << (int)component->GetState() << std::endl;
      return 1;
  }
  
  std::cout << "Step 3: Connecting..." << std::endl;
  simulator.Connect();
  // Confirm component reached CSK_Connected state after simulator Connect.
  if (component->GetState() != Smp::ComponentStateKind::CSK_Connected) {
      std::cerr << "State error after Connect: " << (int)component->GetState() << std::endl;
      return 1;
  }

  std::cout << "Step 4: Connecting Input and Output and testing behavior..." << std::endl;
  
  // Use GetField to find the fields by name
  Smp::ISimpleField* inputField = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input"));
  Smp::ISimpleField* outputField = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output"));

  // Validate both Input and Output simple fields were resolved before access [CS-0010.15].
  if (!inputField || !outputField) {
      std::cerr << "Error: Could not find Input or Output fields via SMP interface!" << std::endl;
      return 1;
  }

  // Find the entry point for execution
  Smp::IEntryPoint* executeEntryPoint = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));
  // Validate Execute entry point exists prior to invocation [CS-0010.15].
  if (!executeEntryPoint) {
      std::cerr << "Error: Could not find Execute entry point via GetChild!" << std::endl;
      return 1;
  }

  // Test 1: Set Input to false, check Output is true after Execute
  inputField->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, false));
  executeEntryPoint->Execute();
  // Verify inversion output is true when Input is false.
  if (!(bool)outputField->GetValue()) {
      std::cerr << "Dynamic test failed: expected true output for false input" << std::endl;
      return 1;
  }
  std::cout << "  Dynamic Test 1 (false -> true) passed." << std::endl;

  // Test 2: Feedback connection (Output -> Input)
  inputField->SetValue(outputField->GetValue()); // Set Input to true (as Output was true)
  executeEntryPoint->Execute();
  // Verify inversion output becomes false when true output is fed back into Input.
  if ((bool)outputField->GetValue()) {
      std::cerr << "Dynamic test failed: expected false output for true input (feedback)" << std::endl;
      return 1;
  }
  std::cout << "  Dynamic Test 2 (true -> false via feedback) passed." << std::endl;

  std::cout << "Dynamic Loading Test completed successfully!" << std::endl;
  return 0;
}
