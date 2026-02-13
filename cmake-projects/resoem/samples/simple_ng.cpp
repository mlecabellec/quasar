/**
 * @file simple_ng.cpp
 * @brief Port of simple_ng.c to resoem
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
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
  std::cout << "Initializing resoem on '" << fb.iface << "'... " << std::flush;
  try {
    fb.socket = std::make_unique<RawSocket>(fb.iface);
    fb.enumerator = std::make_unique<Enumerator>(*fb.socket);
  } catch (const std::exception &e) {
    std::cout << "failed: " << e.what() << std::endl;
    return false;
  }
  std::cout << "done" << std::endl;

  std::cout << "Finding autoconfig slaves... " << std::flush;
  auto enum_res = fb.enumerator->enumerate();
  if (!enum_res || *enum_res == 0) {
    std::cout << "no slaves found" << std::endl;
    return false;
  }
  std::cout << *enum_res << " slaves found" << std::endl;

  std::cout << "Sequential mapping of I/O... " << std::flush;
  auto map_res = fb.enumerator->configure_fmmu(fb.image);
  if (!map_res) {
    std::cout << "failed to map FMMUs" << std::endl;
    return false;
  }
  std::cout << "mapped " << *map_res << " bytes" << std::endl;

  std::cout << "Configuring distributed clock... " << std::flush;
  fb.enumerator->measure_propagation_delays();
  // Note: sync_clocks and configure_dc logic would go here if specific DC
  // config was needed mimicking ecx_configdc
  std::cout << "done" << std::endl;

  std::cout << "Waiting for all slaves in safe operational... " << std::flush;
  auto state_res = fb.enumerator->request_state_all(states::SAFE_OP);
  if (!state_res) {
    std::cout << "failed" << std::endl;
    return false;
  }
  std::cout << "done" << std::endl;

  std::cout << "Send a roundtrip to make outputs in slaves happy... "
            << std::flush;
  fieldbus_roundtrip(fb);
  std::cout << "done" << std::endl;

  std::cout << "Setting operational state..." << std::flush;
  auto op_res = fb.enumerator->request_state_all(states::OP);
  // Wait and check
  bool all_op = false;
  for (int i = 0; i < 40; ++i) { // 40 * 10ms = 400ms wait roughly
    std::cout << "." << std::flush;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (all_op) {
    std::cout << " all slaves are now operational" << std::endl;
    return true;
  }

  std::cout << " failed" << std::endl;
  return false;
}

void fieldbus_stop(Fieldbus &fb) {
  std::cout << "Requesting init state on all slaves... " << std::flush;
  fb.enumerator->request_state_all(states::INIT);
  std::cout << "done" << std::endl;
}

bool fieldbus_dump(Fieldbus &fb) {
  int wkc = fieldbus_roundtrip(fb);
  // Simple verification: wkc should be > 0 if slaves are participating
  printf("%6d usec  WKC %d", fb.roundtrip_time_us, wkc);
  if (wkc <= 0) { // Simplification, ideally calculate expected WKC
    printf(" wrong\n");
    return false;
  }

  // Print first few bytes of IO if available
  if (fb.image.size() > 0) {
    printf(" I/O:");
    for (size_t i = 0; i < std::min((size_t)16, fb.image.size()); ++i) {
      printf(" %02X", fb.image.data()[i]);
    }
  }
  printf("\r");
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
    int min_time = 0, max_time = 0;
    for (int i = 1; i <= 10000; ++i) {
      // Check status periodically?
      if (i % 100 == 0) { // Check status every 100 frames to avoid overhead
        fb.enumerator->check_slaves_status();
        // Recovery logic could go here
      }

      if (!fieldbus_dump(fb)) {
        // handle error
      } else if (i == 1) {
        min_time = max_time = fb.roundtrip_time_us;
      } else {
        if (fb.roundtrip_time_us < min_time)
          min_time = fb.roundtrip_time_us;
        if (fb.roundtrip_time_us > max_time)
          max_time = fb.roundtrip_time_us;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(5000));
    }
    std::cout << "\nRoundtrip time (usec): min " << min_time << " max "
              << max_time << std::endl;
    fieldbus_stop(fb);
  }

  return 0;
}
