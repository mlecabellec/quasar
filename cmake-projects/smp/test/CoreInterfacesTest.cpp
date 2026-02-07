#include "Smp/ISimulator.h"
#include "Smp/Publication/ITypeRegistry.h"
#include "Smp/Services/IEventManager.h"
#include "Smp/Services/ILinkRegistry.h"
#include "Smp/Services/ILogger.h"
#include "Smp/Services/IResolver.h"
#include "Smp/Services/IScheduler.h"
#include "Smp/Services/ITimeKeeper.h"
#include <iostream>

// This test mostly verifies that headers compile correctly and are
// self-contained.
int main() {
  // Step: Start Core Interfaces compilation verification
  std::cout << "Step: Verifying Core Interfaces compilation..." << std::endl;

  // Assertion: Check Logger constant
  std::cout << "Assertion: Checking ILogger::SMP_Logger constant value"
            << std::endl;
  if (Smp::Services::ILogger::SMP_Logger != std::string("Logger")) {
    std::cout << "Error: ILogger::SMP_Logger constant mismatch" << std::endl;
    return 1;
  }

  // Assertion: Check Scheduler constant
  std::cout << "Assertion: Checking IScheduler::SMP_Scheduler constant value"
            << std::endl;
  if (Smp::Services::IScheduler::SMP_Scheduler != std::string("Scheduler")) {
    std::cout << "Error: IScheduler::SMP_Scheduler constant mismatch"
              << std::endl;
    return 1;
  }

  // Assertion: Check TimeKeeper constant
  std::cout << "Assertion: Checking ITimeKeeper::SMP_TimeKeeper constant value"
            << std::endl;
  if (Smp::Services::ITimeKeeper::SMP_TimeKeeper != std::string("TimeKeeper")) {
    std::cout << "Error: ITimeKeeper::SMP_TimeKeeper constant mismatch"
              << std::endl;
    return 1;
  }

  // Final Step: Verification success
  std::cout
      << "Step: Core Interfaces compiled and constants verified successfully."
      << std::endl;
  return 0;
}
