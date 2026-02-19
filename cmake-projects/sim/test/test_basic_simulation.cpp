#include <Smp/IEntryPoint.h>
#include <iostream>
#include <sched/Scheduler.hpp>
#include <sim/Model.hpp>
#include <sim/Simulator.hpp>

#include <core/Object.hpp>

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
    sim::Simulator simulator;

    std::cout << "Simulator created." << std::endl;

    // Add a model
    // We don't have a root model container exposed easily via AddModel unless
    // it expects IModel*. Simulator::AddModel(IModel*) adds to "Models"
    // container.
    auto *model = new MyModel("TestModel", nullptr, &simulator);
    // Note: Model constructor doesn't auto-add to parent if parent is null.
    // And if parent is simulator... Simulator is IObject? Yes.
    // But AddChild?
    // Let's use AddModel.
    simulator.AddModel(model);

    std::cout << "Model added." << std::endl;

    // Lifecycle
    std::cout << "Publishing..." << std::endl;
    simulator.Publish();

    std::cout << "Configuring..." << std::endl;
    simulator.Configure();

    std::cout << "Connecting..." << std::endl;
    simulator.Connect();

    std::cout << "Initialising..." << std::endl;
    simulator.Initialise();

    // Schedule stop event
    StopSimulation stop(&simulator);
    simulator.GetScheduler()->AddSimulationTimeEvent(
        &stop, 1000); // 1 sec? Time unit generic.

    std::cout << "Running..." << std::endl;
    simulator.Run();

    std::cout << "Simulation finished." << std::endl;

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
