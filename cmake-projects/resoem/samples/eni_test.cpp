/**
 * @file eni_test.cpp
 * @brief EtherCAT Network Information (ENI) based configuration test (Resoem).
 * @details Demonstrates how to apply advanced slave configurations (like SDO Init Commands)
 * defined in an ENI XML file (compiled to C via eniconv.py).
 * 
 * Workflow:
 * 1. Enumerate physical slaves.
 * 2. Load ENI structure and verify hardware compatibility.
 * 3. Apply CoE Init Commands defined in ENI.
 * 4. Configure process image and transition to Operational.
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/soem_eni.hpp"
#include "resoem/Diagnostics.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace resoem;

// External reference to the generated ENI structure.
// The variable name matches the ENI XML filename (sanitized).
extern "C" {
extern ec_enit sample_eni;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: eni_test IFNAME [-v]\n";
    return 1;
  }

  std::string iface = argv[1];
  bool verbose = (argc > 2 && std::string(argv[2]) == "-v");

  try {
    std::cout << "======================================================" << std::endl;
    std::cout << "EtherCAT ENI Configuration Test (Resoem)" << std::endl;
    std::cout << "======================================================" << std::endl;

    std::cout << "[STEP 1] Initializing Network on " << iface << "..." << std::endl;
    RawSocket socket(iface);
    Enumerator enumerator(socket);
    enumerator.set_verbose(verbose);

    // 1. Enumerate slaves on network
    std::cout << "[STEP 2] Scanning physical bus..." << std::endl;
    if (Result<size_t> res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "[ERROR] No slaves found. ENI application requires matching hardware.\n";
      return 1;
    }
    std::cout << "  - Found " << enumerator.slaves().size() << " physical slaves.\n";

    // 2. Load ENI Configuration
    std::cout << "[STEP 3] Loading and Verifying ENI data..." << std::endl;
    std::cout << "  - ENI contains configuration for " << sample_eni.slavecount << " slaves.\n";
    
    if (static_cast<size_t>(sample_eni.slavecount) > enumerator.slaves().size()) {
        std::cout << "[WARNING] ENI expects more slaves than found on the bus. This might fail.\n";
    }

    // Apply CoE Init Commands defined in the XML.
    // These commands are typically used to set PDO assignments, mapping, and parameters
    // that must be set in PRE-OP before moving to SAFE-OP/OP.
    std::cout << "[STEP 4] Applying ENI Initialization Commands (CoE)..." << std::endl;
    enumerator.load_eni(&sample_eni);

    // 3. Configure IO mapping
    // Resoem usually reads PDO configuration from SII. 
    // ENI-based masters often use the ENI's PDO mapping instead.
    // For now, we use SII-based FMMU config as implemented in Enumerator.
    std::cout << "[STEP 5] Mapping Process Image (FMMU)..." << std::endl;
    ProcessImage image;
    Result<uint32_t> map_res = enumerator.configure_fmmu(image);
    if (!map_res) {
        std::cerr << "  [ERROR] FMMU mapping failed.\n";
        return 1;
    }
    std::cout << "  - Mapped " << image.size() << " bytes of process data.\n";

    // 4. Request SafeOp
    std::cout << "[STEP 6] Transitioning to SAFE-OP..." << std::endl;
    if (!enumerator.request_state_all(states::SAFE_OP)) {
        std::cerr << "  [ERROR] Failed to reach SAFE-OP state.\n";
        return 1;
    }

    // 5. Check State
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    enumerator.check_slaves_status();

    bool all_safe_op = true;
    for (size_t i = 0; i < enumerator.slaves().size(); ++i) {
      if (enumerator.slaves()[i].current_state != states::SAFE_OP) {
          std::cout << "  - Slave " << i << " is NOT in SAFE-OP (State: 0x" 
                    << std::hex << enumerator.slaves()[i].current_state << std::dec << ")\n";
          all_safe_op = false;
      }
    }

    if (all_safe_op) {
      std::cout << "[SUCCESS] All slaves reached SAFE-OP.\n";

      std::cout << "[STEP 7] Requesting OPERATIONAL..." << std::endl;
      enumerator.request_state_all(states::OP);

      std::cout << "[STEP 8] Starting Process Data Loop (500 cycles)..." << std::endl;
      for (int i = 0; i < 500; ++i) {
        enumerator.exchange_process_data(image);
        if (i % 100 == 0) {
          enumerator.check_slaves_status();
          bool op = true;
          for (const SlaveInfo &s : enumerator.slaves()) {
            if (s.current_state != states::OP) op = false;
          }
          if (op)
            std::cout << "  - Cycle " << i << ": All Slaves OP\r" << std::flush;
          else
            std::cout << "  - Cycle " << i << ": Waiting for OP...\r" << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }
      std::cout << "\n[INFO] Loop finished.\n";
    } else {
      std::cout << "[ERROR] Hardware mismatch or invalid InitCmds in ENI.\n";
    }

    std::cout << "[STEP 9] Reverting to INIT state..." << std::endl;
    enumerator.request_state_all(states::INIT);
    
    std::cout << "======================================================" << std::endl;
    std::cout << "ENI Test Finished." << std::endl;
    std::cout << "======================================================" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "[FATAL] " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
