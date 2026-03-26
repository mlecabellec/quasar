/**
 * @file simple_ng.cpp
 * @brief Port of simple_ng.c to resoem with enhanced verbosity and debug info.
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Diagnostics.hpp"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace resoem;

struct Fieldbus {
  std::unique_ptr<RawSocket> socket;
  std::unique_ptr<Enumerator> enumerator;
  ProcessImage image;
  std::string iface;
  int roundtrip_time_us = 0;
};

int fieldbus_roundtrip(Fieldbus &fb) {
  auto start = std::chrono::steady_clock::now();
  auto res = fb.enumerator->exchange_process_data(fb.image);
  auto end = std::chrono::steady_clock::now();

  fb.roundtrip_time_us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  if (!res)
    return -1;
  return *res;
}

bool fieldbus_start(Fieldbus &fb) {
  std::cout << "======================================================" << std::endl;
  std::cout << "Starting EtherCAT Master on interface: " << fb.iface << std::endl;
  std::cout << "======================================================" << std::endl;

  try {
    std::cout << "[STEP 1] Initializing Raw Socket..." << std::endl;
    fb.socket = std::make_unique<RawSocket>(fb.iface);
    
    std::cout << "[STEP 2] Initializing Enumerator..." << std::endl;
    fb.enumerator = std::make_unique<Enumerator>(*fb.socket);
    
    // Enable extensive debug logging
    fb.enumerator->set_verbose(true);
    std::cout << "[INFO] Verbose debug logging ENABLED." << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "[ERROR] Initialization failed: " << e.what() << std::endl;
    return false;
  }

  std::cout << "\n[STEP 3] Discovering and Counting Slaves..." << std::endl;
  auto enum_res = fb.enumerator->enumerate();
  if (!enum_res) {
    std::cerr << "[ERROR] Enumeration failed: " << enum_res.error() << std::endl;
    return false;
  }
  
  size_t slave_count = *enum_res;
  if (slave_count == 0) {
    std::cerr << "[ERROR] No EtherCAT slaves detected on the bus!" << std::endl;
    return false;
  }
  std::cout << "[SUCCESS] Found " << slave_count << " slaves." << std::endl;

  std::cout << "\n[STEP 4] Configuring FMMU (Logical Memory Mapping)..." << std::endl;
  auto map_res = fb.enumerator->configure_fmmu(fb.image);
  if (!map_res) {
    std::cerr << "[ERROR] FMMU configuration failed!" << std::endl;
    return false;
  }
  std::cout << "[SUCCESS] Process Image mapped: " << *map_res << " bytes." << std::endl;

  std::cout << "\n[STEP 5] Measuring Bus Propagation Delays (DC)..." << std::endl;
  fb.enumerator->measure_propagation_delays();
  std::cout << "[INFO] DC propagation delays measured and compensated." << std::endl;

  std::cout << "\n[STEP 6] Transitioning all slaves to SAFE-OP..." << std::endl;
  auto state_res = fb.enumerator->request_state_all(states::SAFE_OP);
  if (!state_res) {
    std::cerr << "[ERROR] Failed to reach SAFE-OP state!" << std::endl;
    return false;
  }
  std::cout << "[SUCCESS] All slaves in SAFE-OP." << std::endl;

  std::cout << "\n[STEP 7] Initial Cyclic Data Exchange (Warming up)..." << std::endl;
  fieldbus_roundtrip(fb);
  std::cout << "[INFO] Initial roundtrip completed." << std::endl;

  std::cout << "\n[STEP 8] Transitioning all slaves to OPERATIONAL..." << std::endl;
  auto op_res = fb.enumerator->request_state_all(states::OP);
  
  std::cout << "Waiting for slaves to sync to OP state..." << std::flush;
  bool all_op = false;
  for (int i = 0; i < 100; ++i) { // Increase retry count
    fieldbus_roundtrip(fb);
    fb.enumerator->check_slaves_status();

    all_op = true;
    for (const auto &slave : fb.enumerator->slaves()) {
      if (slave.current_state != states::OP) {
        all_op = false;
        break;
      }
    }
    if (all_op)
      break;
    
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::cout << std::endl;

  if (all_op) {
    std::cout << "[SUCCESS] All slaves are now OPERATIONAL and exchanging data." << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    return true;
  }

  std::cerr << "[ERROR] Some slaves failed to reaching OP state!" << std::endl;
  for (size_t i = 0; i < fb.enumerator->slaves().size(); ++i) {
      const auto& s = fb.enumerator->slaves()[i];
      if (s.current_state != states::OP) {
          std::cerr << "  - Slave " << i << " (" << s.name << ") is in state 0x" 
                    << std::hex << s.current_state << std::dec << std::endl;
      }
  }
  return false;
}

void fieldbus_stop(Fieldbus &fb) {
  std::cout << "\n[SHUTDOWN] Requesting INIT state on all slaves..." << std::endl;
  fb.enumerator->request_state_all(states::INIT);
  std::cout << "[INFO] Shutdown complete." << std::endl;
}

bool fieldbus_dump(Fieldbus &fb) {
  int wkc = fieldbus_roundtrip(fb);
  
  if (wkc < 0) {
    std::cerr << "\n[ERROR] Bus communication timeout!" << std::endl;
    return false;
  }

  printf("%6d us | WKC %d", fb.roundtrip_time_us, wkc);
  
  if (wkc <= 0) {
    printf(" [MISMATCH]");
  }

  if (fb.image.size() > 0) {
    printf(" | IO: ");
    for (size_t i = 0; i < std::min((size_t)8, fb.image.size()); ++i) {
      printf("%02X ", fb.image.data()[i]);
    }
  }
  printf("\r");
  fflush(stdout);
  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: simple_ng IFNAME" << std::endl;
    return 1;
  }

  Fieldbus fb;
  fb.iface = argv[1];

  if (fieldbus_start(fb)) {
    std::cout << "Running cyclic loop (Press Ctrl+C to stop)..." << std::endl;
    
    int min_time = 999999, max_time = 0;
    for (int i = 1; i <= 10000; ++i) {
      if (i % 500 == 0) {
        fb.enumerator->check_slaves_status();
        for (const auto& s : fb.enumerator->slaves()) {
            if (!s.online || s.current_state != states::OP) {
                std::cerr << "\n[ALERT] Slave status changed! Slave " << s.configured_address 
                          << " is " << (s.online ? "ONLINE" : "OFFLINE") 
                          << " state=0x" << std::hex << s.current_state << std::dec << std::endl;
            }
        }
      }

      if (!fieldbus_dump(fb)) {
        std::cerr << "\n[FATAL] Bus error detected. Stopping." << std::endl;
        break;
      }

      if (fb.roundtrip_time_us < min_time) min_time = fb.roundtrip_time_us;
      if (fb.roundtrip_time_us > max_time) max_time = fb.roundtrip_time_us;
      
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    
    std::cout << "\n\nLoop Finished." << std::endl;
    std::cout << "Roundtrip time (us): min=" << min_time << " max=" << max_time << std::endl;
    fieldbus_stop(fb);
  }

  return 0;
}
