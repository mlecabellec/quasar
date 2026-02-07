#include "Smp/Simulator.h"
#include "Smp/Publication/TypeRegistry.h"
#include <cassert>
#include <iostream>

int main() {
  std::cout << "Step: Initialize TypeRegistry and Simulator" << std::endl;
  Smp::Publication::TypeRegistry *registry =
      new Smp::Publication::TypeRegistry();
  Smp::Simulator *simulator =
      new Smp::Simulator("Sim", "Test Simulator", registry);

  // Test 1: Check initial state
  std::cout << "Test 1: Check initial state (Building)" << std::endl;
  if (simulator->GetState() != Smp::SimulatorStateKind::SSK_Building) {
    std::cout << "Error: Expected state SSK_Building, got "
              << (int)simulator->GetState() << std::endl;
    return 1;
  }

  // Test 2: Add Model
  std::cout << "Test 2: Add and verify model container" << std::endl;
  auto *models = simulator->GetContainer(Smp::ISimulator::SMP_SimulatorModels);
  if (models == nullptr) {
    std::cout << "Error: Models container not found" << std::endl;
    return 1;
  }
  std::cout << "Models container found." << std::endl;

  // Test 3: Transitions
  std::cout << "Test 3: Verify state transitions" << std::endl;

  std::cout << "Step: Connect()" << std::endl;
  simulator->Connect();
  if (simulator->GetState() != Smp::SimulatorStateKind::SSK_Standby) {
    std::cout << "Error: Expected state SSK_Standby after Connect(), got "
              << (int)simulator->GetState() << std::endl;
    return 1;
  }

  std::cout << "Step: Run()" << std::endl;
  simulator->Run();
  if (simulator->GetState() != Smp::SimulatorStateKind::SSK_Executing) {
    std::cout << "Error: Expected state SSK_Executing after Run(), got "
              << (int)simulator->GetState() << std::endl;
    return 1;
  }

  std::cout << "Step: Hold()" << std::endl;
  simulator->Hold(true);
  if (simulator->GetState() != Smp::SimulatorStateKind::SSK_Standby) {
    std::cout << "Error: Expected state SSK_Standby after Hold(), got "
              << (int)simulator->GetState() << std::endl;
    return 1;
  }

  std::cout << "Test 3 Passed: Transitions verified." << std::endl;

  delete simulator;
  delete registry;

  std::cout << "All Simulator tests passed!" << std::endl;
  return 0;
}
