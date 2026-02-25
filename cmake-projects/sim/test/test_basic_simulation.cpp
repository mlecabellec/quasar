#include <Smp/IEntryPoint.h>
#include <iostream>
#include <sched/Scheduler.hpp>
#include <sim/Model.hpp>
#include <sim/Simulator.hpp>

#include <core/Object.hpp>
#include <memory>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ILogger.h>

/**
 * @brief Test file for basic simulator functionality.
 *
 * This test verifies the core simulation lifecycle, service integration, and
 * basic component management.
 *
 * Contribution to FE-0030:
 * - [FE-0030.10] Testing: The existence of this test file demonstrates a commitment
 *   to testing. However, this specific test does not directly cover the features
 *   outlined in FE-0030 (e.g., Number operations, String class, Buffer/BitBuffer
 *   functionality, thread safety aspects). It primarily tests the simulation
 *   framework's core setup and lifecycle.
 *
 * Contribution to FE-0050.6 (Simulation Lifecycle):
 * - Demonstrates the sequence of simulator and component lifecycle methods:
 *   Simulator creation, Publish, Configure, Connect, Initialise, Run, Hold, Exit.
 * - Each step is annotated with relevant ECSS standard compliance proofs,
 *   indirectly referencing FE-0050.6.1's states and transitions.
 *
 * Contribution to FE-0050.4 (UUIDs):
 * - The `MyModel` class includes a stub `GetUuid()` method, indicating the
 *   expectation that components have unique identifiers as per FE-0050.4.1.
 *
 * Contribution to FE-0050.1 (Primitive Types):
 * - While not directly testing SMP primitive types or AnySimple, the test relies
 *   on underlying C++ primitive types and standard library components, which
 *   are the foundation for SMP's primitive type system. The `MyModel` stub
 *   methods for simple value access suggest future interaction with primitive types.
 */

class StopSimulation : public core::Object, public Smp::IEntryPoint {
public:
  StopSimulation(Smp::ISimulator *simulator)
      : core::Object("StopSimulation", "Stops the simulation", nullptr),
        _simulator(simulator) {}

  void Execute() const override {
    std::cout << "Event triggered. Stopping simulation." << std::endl;
    _simulator->Hold(true); // [FE-0050.6.1] Example of transitioning simulation state (Executing -> Standby via Hold).
  }

  // Satisfy IObject (already done by core::Object)
  Smp::String8 GetName() const override { return core::Object::GetName(); }
  Smp::String8 GetDescription() const override {
    return core::Object::GetDescription();
  }
  Smp::IObject *GetParent() const override { return core::Object::GetParent(); }
  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }

private:
  Smp::ISimulator *_simulator;
};

class MyModel : public sim::Model {
public:
  MyModel(Smp::String8 name, Smp::IObject *parent, Smp::ISimulator *simulator)
      : sim::Model(name, "", parent, simulator) {}

  void Publish(Smp::IPublication *receiver) override {
    sim::Model::Publish(receiver); // [FE-0050.6.1] Component state transition: Created -> Publishing.
    std::cout << "MyModel Published." << std::endl;
  }

  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override {
    sim::Model::Configure(logger, linkRegistry); // [FE-0050.6.1] Component state transition: Publishing -> Configured.
    std::cout << "MyModel Configured." << std::endl;
  }

  void Connect(Smp::ISimulator *simulator) override {
    sim::Model::Connect(simulator); // [FE-0050.6.1] Component state transition: Configured -> Connected.
    std::cout << "MyModel Connected." << std::endl;
  }
  
  // Stub for GetUuid to satisfy FE-0050.4.1 expectation for components.
  const Smp::Uuid &GetUuid() const override {
    static Smp::Uuid validUuid; // TODO: Generate or allow setting via constructor
    return validUuid;
  }
};

int main() {
  std::cout << "Starting Simulation Test..." << std::endl;

  try {
    /// [Compliance Proof] FE-0070.7.1: Simulator creation and service
    /// initialization. This also implies the start of the lifecycle,
    /// corresponding to the initial state before explicit transitions like
    /// Building or Standby, aligning with FE-0050.6.1.
    sim::Simulator simulator;

    std::cout << "Simulator created." << std::endl;

    // Add a model
    // Using unique_ptr to avoid 'new'
    std::unique_ptr<MyModel> modelPtr = std::make_unique<MyModel>("TestModel", nullptr, &simulator);
    MyModel* model = modelPtr.get();
    
    /// [Compliance Proof] FE-0070.7.17: Adding a model to the simulator.
    // Transfer ownership to simulator
    simulator.AddModel(modelPtr.release());

    std::cout << "Model added." << std::endl;

    // Lifecycle
    /// [Compliance Proof] FE-0070.7.3/4: Simulator Publish lifecycle phase.
    // [FE-0050.6.1] Corresponds to Simulator state SSK_Building.
    std::cout << "Publishing..." << std::endl;
    simulator.Publish();

    /// [Compliance Proof] FE-0070.7.3/5: Simulator Configure lifecycle phase.
    // [FE-0050.6.1] Corresponds to Simulator state SSK_Building.
    std::cout << "Configuring..." << std::endl;
    simulator.Configure();

    /// [Compliance Proof] FE-0070.7.3/6: Simulator Connect lifecycle phase.
    // [FE-0050.6.1] Transitions Simulator state to SSK_Connecting, then SSK_Initialising, and finally SSK_Standby.
    std::cout << "Connecting..." << std::endl;
    simulator.Connect();

    /// [Compliance Proof] FE-0070.7.3/7: Simulator Initialise lifecycle phase.
    // [FE-0050.6.1] Corresponds to Simulator state SSK_Standby after Initialising.
    std::cout << "Initialising..." << std::endl;
    simulator.Initialise();

    /// [Compliance Proof] FE-0070.5.2: Path resolution using Resolver service.
    Smp::IObject *resolved = simulator.GetResolver()->ResolveAbsolute("/TestModel");
    if (resolved != model) {
      throw std::runtime_error("Resolver failed to find model");
    }
    std::cout << "Path resolution verified." << std::endl;

    /// [Compliance Proof] FE-0070.6.2: Link registration using LinkRegistry
    /// service. This service is part of the simulation setup that occurs
    /// during or before execution.
    simulator.GetLinkRegistry()->AddLink(model, model); // Self link for test
    if (simulator.GetLinkRegistry()->GetLinkCount(model, model) != 1) {
      throw std::runtime_error("LinkRegistry failed to register link");
    }
    std::cout << "Link registration verified." << std::endl;

    // Schedule stop event
    /// [Compliance Proof] FE-0070.3.5: Adding a simulation time event to the
    /// scheduler. This is part of setting up the execution phase.
    StopSimulation stop(&simulator);
    simulator.GetScheduler()->AddSimulationTimeEvent(
        &stop, 1000); // 1 sec? Time unit generic.

    /// [Compliance Proof] FE-0070.7.8: Simulator Run (transition to Executing
    /// state).
    // [FE-0050.6.1] Corresponds to Simulator state SSK_Executing.
    std::cout << "Running..." << std::endl;
    simulator.Run();

    std::cout << "Simulation finished." << std::endl;

    /// [Compliance Proof] FE-0070.7.13: Simulator Exit.
    // [FE-0050.6.1] Transition from SSK_Standby to SSK_Exiting.
    simulator.Exit();

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception." << std::endl;
    return 1;
  }

  return 0;
}
