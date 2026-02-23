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

class StopSimulation : public core::Object, public Smp::IEntryPoint {
public:
  StopSimulation(Smp::ISimulator *simulator)
      : core::Object("StopSimulation", "Stops the simulation", nullptr),
        _simulator(simulator) {}

  void Execute() const override {
    std::cout << "Event triggered. Stopping simulation." << std::endl;
    _simulator->Hold(true);
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
    sim::Model::Publish(receiver);
    std::cout << "MyModel Published." << std::endl;
  }

  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry) override {
    sim::Model::Configure(logger, linkRegistry);
    std::cout << "MyModel Configured." << std::endl;
  }

  void Connect(Smp::ISimulator *simulator) override {
    sim::Model::Connect(simulator);
    std::cout << "MyModel Connected." << std::endl;
  }
};

int main() {
  std::cout << "Starting Simulation Test..." << std::endl;

  try {
    /// [Compliance Proof] FE-0070.7.1: Simulator creation and service
    /// initialization.
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
    std::cout << "Publishing..." << std::endl;
    simulator.Publish();

    /// [Compliance Proof] FE-0070.7.3/5: Simulator Configure lifecycle phase.
    std::cout << "Configuring..." << std::endl;
    simulator.Configure();

    /// [Compliance Proof] FE-0070.7.3/6: Simulator Connect lifecycle phase.
    std::cout << "Connecting..." << std::endl;
    simulator.Connect();

    /// [Compliance Proof] FE-0070.7.3/7: Simulator Initialise lifecycle phase.
    std::cout << "Initialising..." << std::endl;
    simulator.Initialise();

    /// [Compliance Proof] FE-0070.5.2: Path resolution using Resolver service.
    Smp::IObject *resolved = simulator.GetResolver()->ResolveAbsolute("/TestModel");
    if (resolved != model) {
      throw std::runtime_error("Resolver failed to find model");
    }
    std::cout << "Path resolution verified." << std::endl;

    /// [Compliance Proof] FE-0070.6.2: Link registration using LinkRegistry
    /// service.
    simulator.GetLinkRegistry()->AddLink(model, model); // Self link for test
    if (simulator.GetLinkRegistry()->GetLinkCount(model, model) != 1) {
      throw std::runtime_error("LinkRegistry failed to register link");
    }
    std::cout << "Link registration verified." << std::endl;

    // Schedule stop event
    /// [Compliance Proof] FE-0070.3.5: Adding a simulation time event to the
    /// scheduler.
    StopSimulation stop(&simulator);
    simulator.GetScheduler()->AddSimulationTimeEvent(
        &stop, 1000); // 1 sec? Time unit generic.

    /// [Compliance Proof] FE-0070.7.8: Simulator Run (transition to Executing
    /// state).
    std::cout << "Running..." << std::endl;
    simulator.Run();

    std::cout << "Simulation finished." << std::endl;

    /// [Compliance Proof] FE-0070.7.13: Simulator Exit.
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
