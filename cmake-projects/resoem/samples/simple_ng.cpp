/**
 * @file simple_ng.cpp
 * @brief Enhanced EtherCAT cyclic data exchange sample (Resoem).
 * @details This sample demonstrates a full EtherCAT lifecycle:
 * 1. Network initialization and slave discovery.
 * 2. Logical memory mapping (FMMU).
 * 3. State transitions (Init -> PreOp -> SafeOp -> Operational).
 * 4. High-frequency cyclic process data exchange with health monitoring.
 * 5. Graceful shutdown.
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
#include <signal.h>

using namespace resoem;

// Global flag for graceful exit on Ctrl+C
volatile sig_atomic_t g_stop = 0;
void handle_sigint(int) { g_stop = 1; }

/**
 * @brief Encapsulates the fieldbus state and context.
 */
struct Fieldbus;

/**
 * @brief Helper to get detailed error info for a slave.
 */
std::string get_diagnostics(Fieldbus &fb, int slave_idx);

/**
 * @brief Encapsulates the fieldbus state and context.
 */
struct Fieldbus {
  std::unique_ptr<RawSocket> socket;
  std::unique_ptr<Enumerator> enumerator;
  ProcessImage image;
  std::string iface;
  int roundtrip_time_us = 0;
  uint16_t expected_wkc = 0;
};

std::string get_diagnostics(Fieldbus &fb, int slave_idx);

/**
 * @brief Performs a single process data exchange roundtrip.
 */
int fieldbus_roundtrip(Fieldbus &fb) {
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  Result<uint16_t> res = fb.enumerator->exchange_process_data(fb.image);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  fb.roundtrip_time_us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  if (!res)
    return -1; // Timeout or internal error
  return *res; // Return actual Working Counter
}

/**
 * @brief Orchestrates the startup sequence.
 */
bool fieldbus_start(Fieldbus &fb, int verbose_level) {
  std::cout << "======================================================" << std::endl;
  std::cout << "Starting EtherCAT Master on: " << fb.iface << std::endl;
  std::cout << "======================================================" << std::endl;

  try {
    std::cout << "[STEP 1] Opening Raw Socket (AF_PACKET)..." << std::endl;
    fb.socket = std::make_unique<RawSocket>(fb.iface);
    
    std::cout << "[STEP 2] Initializing Enumerator..." << std::endl;
    fb.enumerator = std::make_unique<Enumerator>(*fb.socket);
    fb.enumerator->set_verbose(verbose_level);

    std::cout << "[STEP 3] Scanning Bus..." << std::endl;
    Result<size_t> enum_res = fb.enumerator->enumerate();
    if (!enum_res) {
      std::cerr << "  [ERROR] Enumeration failed: " << static_cast<int>(enum_res.error()) << std::endl;
      return false;
    }
    
    size_t slave_count = *enum_res;
    if (slave_count == 0) {
      std::cerr << "  [ERROR] No slaves detected. Check cable/power." << std::endl;
      return false;
    }
    std::cout << "  [INFO] Found " << slave_count << " slaves." << std::endl;

    std::cout << "[STEP 4] Configuring Process Data Mappings (FMMU)..." << std::endl;
    Result<uint32_t> map_res = fb.enumerator->configure_fmmu(fb.image);
    if (!map_res) {
      std::cerr << "  [ERROR] FMMU configuration failed!" << std::endl;
      return false;
    }
    std::cout << "  [INFO] Process Image size: " << *map_res << " bytes." << std::endl;

    // Calculate the expected Working Counter (WKC) for the LRW cyclic command.
    // Per ETG.1000.4 §5.4.7 and IgH master datagram_pair.c:87-120:
    //   - Each slave with OUTPUTS (Master -> Slave) contributes +1 to LRW WKC.
    //     The slave *reads* from the logical image; a successful read increments WKC by 1.
    //   - Each slave with INPUTS (Slave -> Master) contributes +2 to LRW WKC.
    //     The slave *writes* to the logical image; a successful write increments WKC by 2.
    //   - A slave with both directions active contributes +3 total.
    // These constants are fixed by the EtherCAT standard for LRW commands. [CS-0010.39]
    constexpr uint16_t WKC_LRW_OUTPUT_INCREMENT = 1U;  // Slave reads logical image
    constexpr uint16_t WKC_LRW_INPUT_INCREMENT  = 2U;  // Slave writes logical image

    fb.expected_wkc = 0;
    for (const SlaveInfo& s : fb.enumerator->slaves()) {
        // Accumulate WKC contributions from each slave that participates in process data.
        if (s.outputs_size_bits > 0) { fb.expected_wkc += WKC_LRW_OUTPUT_INCREMENT; }
        if (s.inputs_size_bits  > 0) { fb.expected_wkc += WKC_LRW_INPUT_INCREMENT; }
    }
    std::cout << "  [INFO] Expected Working Counter (WKC): " << fb.expected_wkc << std::endl;

    std::cout << "[STEP 5] Distributed Clocks (DC) Setup..." << std::endl;
    fb.enumerator->measure_propagation_delays();
    std::cout << "  [INFO] Propagation delays compensated." << std::endl;

    std::cout << "[STEP 6] Transitioning to SAFE-OP (Synchronizing)..." << std::endl;
    if (!fb.enumerator->request_state_all(states::SAFE_OP)) {
      std::cerr << "  [ERROR] Some slaves failed to reach SAFE-OP." << std::endl;
      for (size_t i=0; i<fb.enumerator->slaves().size(); ++i) {
          const SlaveInfo& s = fb.enumerator->slaves()[i];
          if (s.current_state != states::SAFE_OP)
              std::cerr << "    - Slave " << i << " (" << s.name << ") is in " << get_diagnostics(fb, (int)i) << std::endl;
      }
      return false;
    }

    std::cout << "[STEP 7] Warm-up roundtrips..." << std::endl;
    for(int i=0; i<10; ++i) fieldbus_roundtrip(fb);

    std::cout << "[STEP 8] Transitioning to OPERATIONAL..." << std::endl;
    fb.enumerator->request_state_all(states::OP);
    
    // Wait for OP state with monitor
    bool all_op = false;
    for (int i = 0; i < 50; ++i) {
      fieldbus_roundtrip(fb);
      fb.enumerator->check_slaves_status();
      all_op = true;
      for (const SlaveInfo &s : fb.enumerator->slaves()) {
        if (s.current_state != states::OP) { all_op = false; break; }
      }
      if (all_op) break;
      std::cout << "." << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << std::endl;

    if (!all_op) {
      std::cerr << "  [ERROR] Time-out waiting for Operational state." << std::endl;
      for (size_t i=0; i<fb.enumerator->slaves().size(); ++i) {
          SlaveInfo s = fb.enumerator->slaves()[i];
          if (s.current_state != states::OP)
              std::cerr << "    - Slave " << i << " (" << s.name << ") is in " << get_diagnostics(fb, i) << std::endl;
      }
      return false;
    }

    std::cout << "[SUCCESS] Fieldbus is LIVE." << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "[FATAL] Initialization exception: " << e.what() << std::endl;
    return false;
  }
}

/**
 * @brief Helper to get detailed error info for a slave.
 */
std::string get_diagnostics(Fieldbus &fb, int slave_idx) {
    int wkc;
    uint16_t code = fb.enumerator->read_register_fprd<uint16_t>(
        fb.enumerator->slaves()[slave_idx].configured_address, regs::AL_STATUS_CODE, wkc);
    if (wkc > 0 && code != 0) {
        return "Error 0x" + std::to_string(code) + ": " + std::string(al_status_code_to_string(code));
    }
    return "State 0x" + std::to_string(fb.enumerator->slaves()[slave_idx].current_state);
}

/**
 * @brief Shutdown sequence.
 */
void fieldbus_stop(Fieldbus &fb) {
  std::cout << "\n[SHUTDOWN] Reverting all slaves to INIT..." << std::endl;
  fb.enumerator->request_state_all(states::INIT);
  std::cout << "[INFO] Goodbye." << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: simple_ng IFNAME [-v | -vv]" << std::endl;
    return 1;
  }

  signal(SIGINT, handle_sigint);
  Fieldbus fb;
  fb.iface = argv[1];
  
  int verbose_level = 0;
  if (argc > 2) {
    std::string verbosity_flag = argv[2];
    if (verbosity_flag == "-v") {
      verbose_level = 1;
    } else if (verbosity_flag == "-vv") {
      verbose_level = 2;
    }
  }

  if (fieldbus_start(fb, verbose_level)) {
    std::cout << "Running cyclic loop. Press Ctrl+C to stop." << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    
    uint64_t cycle = 0;
    while (!g_stop) {
      int wkc = fieldbus_roundtrip(fb);
      cycle++;

      // Periodic health check (every 500 cycles)
      if (cycle % 500 == 0) {
        fb.enumerator->check_slaves_status();
        for (size_t i=0; i<fb.enumerator->slaves().size(); ++i) {
            const SlaveInfo& s = fb.enumerator->slaves()[i];
            if (!s.online || s.current_state != states::OP) {
                std::cerr << "\n[ALERT] Slave " << i << " (" << s.name << ") HEALTH ISSUE!" << std::endl;
                std::cerr << "        Status: " << (s.online ? "ONLINE" : "OFFLINE") 
                          << " | State: " << get_diagnostics(fb, i) << std::endl;
            }
        }
      }

      // Display live stats
      if (cycle % 10 == 0) {
          printf("\rCycle: %8lu | Latency: %5d us | WKC: %3d/%3d", 
                 cycle, fb.roundtrip_time_us, wkc, fb.expected_wkc);
          if (wkc != fb.expected_wkc) printf(" [BAD]");
          else printf(" [OK] ");
          
          if (fb.image.size() > 0) {
              printf(" | Data[0..3]: %02X %02X %02X %02X", 
                     fb.image.data()[0], fb.image.data()[1], 
                     fb.image.data()[2], fb.image.data()[3]);
          }
          fflush(stdout);
      }

      if (wkc < 0) {
          std::cerr << "\n[FATAL] Bus Communication Timeout! Check link." << std::endl;
          break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    
    std::cout << "\n------------------------------------------------------" << std::endl;
    std::cout << "Loop stopped after " << cycle << " cycles." << std::endl;
    fieldbus_stop(fb);
  }

  return 0;
}
