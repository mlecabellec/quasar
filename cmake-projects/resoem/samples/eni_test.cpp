/**
 * @file eni_test.cpp
 * @brief Port of eni_test.c to resoem
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/soem_eni.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace resoem;

// External reference to the generated ENI structure
// The name depends on the ENI XML filename.
// sample-eni.xml -> sample_eni
extern "C" {
extern ec_enit sample_eni;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: eni_test IFNAME\n";
    return 1;
  }

  std::string iface = argv[1];

  try {
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    std::cout << "ENI Test (Resoem)\n";

    // 1. Enumerate slaves on network
    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "No slaves found. ENI application might fail if slaves "
                   "don't match.\n";
      // Continue? Or exit?
      return 1;
    }

    // 2. Load ENI Configuration
    // This applies CoE Init Commands defined in the XML
    enumerator.load_eni(&sample_eni);

    // 3. Configure IO mapping (PDOs from ENI?)
    // Note: Resoem usually reads PDO config from SII (read_sii_pdos).
    // ENI also contains PDO config.
    // Enumerator::enumerate() already read SII.
    // If we want to use ENI PDO config, we would need to override
    // slaves_[i].rx_pdos/tx_pdos with data from ENI. Our current load_eni
    // implementation only does CoE Init Cmds. For this test, we rely on SII for
    // PDO mapping as Resoem default, assuming ENI mainly provides CoE Init Cmds
    // (SafeOp config).

    ProcessImage image;
    enumerator.configure_fmmu(image);
    std::cout << "Mapped " << image.size() << " bytes of process data.\n";

    // 4. Request SafeOp
    enumerator.request_state_all(states::SAFE_OP);

    // 5. Check State
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    enumerator.check_slaves_status();

    bool all_safe_op = true;
    for (const auto &s : enumerator.slaves()) {
      if (s.current_state != states::SAFE_OP)
        all_safe_op = false;
    }

    if (all_safe_op) {
      std::cout << "All slaves reached SafeOpt.\n";

      std::cout << "Requesting Operational...\n";
      enumerator.request_state_all(states::OP);

      // PDO Loop
      for (int i = 0; i < 500; ++i) {
        enumerator.exchange_process_data(image);
        if (i % 50 == 0) {
          enumerator.check_slaves_status();
          bool op = true;
          for (const auto &s : enumerator.slaves()) {
            if (s.current_state != states::OP)
              op = false;
          }
          if (op)
            std::cout << "All Slaves OP\r" << std::flush;
          else
            std::cout << "Waiting for OP...\r" << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }
      std::cout << "\nDone.\n";
    } else {
      std::cout << "Failed to reach SafeOp. Check InitCmds.\n";
    }

    enumerator.request_state_all(states::INIT);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
