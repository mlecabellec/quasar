/**
 * @file slaveinfo.cpp
 * @brief Port of slaveinfo.c to resoem
 */

#include "resoem/CoEHandler.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace resoem;

void print_slave_info(const SlaveInfo &s, int idx) {
  std::cout << "Slave:" << idx + 1 << "\n";
  std::cout << " Name: " << s.name << "\n";
  std::cout << " Output size: " << s.outputs_size_bits << " bits\n";
  std::cout << " Input size: " << s.inputs_size_bits << " bits\n";
  std::cout << " Configured Addr: " << std::hex << s.configured_address
            << std::dec << "\n";
  std::cout << " Vendor ID: 0x" << std::hex << s.vendor_id << "\n";
  std::cout << " Product Code: 0x" << s.product_code << std::dec << "\n";

  // Print FMMUs
  for (size_t i = 0; i < s.fmmu.size(); ++i) {
    const auto &f = s.fmmu[i];
    std::cout << " FMMU" << i << ": LogStart=0x" << std::hex
              << f.logical_start << " Len=" << f.length << " Phys=0x"
              << f.physical_start << " Type=" << (int)f.type << std::dec
              << "\n";
  }

  // Print SMs
  for (size_t i = 0; i < s.sync_managers.size(); ++i) {
    const auto &sm = s.sync_managers[i];
    std::cout << " SM" << i << ": Phys=0x" << std::hex << sm.start_addr
              << " Len=" << sm.length << " Type=" << (int)sm.type << " Flags=0x"
              << sm.flags << std::dec << "\n";
  }
}

void print_coe_od(Enumerator &enumerator, int slave_idx) {
  auto &slave = const_cast<SlaveInfo &>(enumerator.slaves()[slave_idx]);
  (void)slave; // Fix unused variable warning
  // We need MailboxHandler/CoEHandler
  // Enumerator doesn't expose them directly, but we can construct them?
  // Wait, Enumerator takes RawSocket&. We need access to that socket.
  // We cannot access private member socket_.
  // BUT, we passed the socket to Enumerator constructor. We still have the
  // socket object in main context. However, Enumerator claims the socket (it's
  // a referece). Thread safety: As long as we are single threaded it's fine.
  // But we need the socket object.

  // Issue: Enumerator holds RawSocket&. We need it.
  // Solution: main passes RawSocket to both Enumerator and CoEHandler?
  // Yes.
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: slaveinfo IFNAME [-smap] [-od]" << std::endl;
    return 1;
  }

  std::string iface = argv[1];
  bool print_map = false;
  (void)print_map; // Fix unused variable warning
  bool print_od = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-smap")
      print_map = true;
    if (arg == "-od")
      print_od = true;
  }

  try {
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    std::cout << "SOEM (Simple Open EtherCAT Master)\nSlaveinfo\nStarting "
                 "enumeration...\n";
    auto count_res = enumerator.enumerate();

    if (!count_res || *count_res == 0) {
      std::cout << "No slaves found.\n";
      return 0;
    }

    std::cout << *count_res << " slaves found.\n";

    // Needed to map inputs/outputs for size calculation, even if we don't
    // exchange data
    ProcessImage image;
    enumerator.configure_fmmu(image);

    enumerator.request_state_all(states::SAFE_OP);
    // Wait for state
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    enumerator.check_slaves_status();

    for (size_t i = 0; i < enumerator.slaves().size(); ++i) {
      print_slave_info(enumerator.slaves()[i], i);

      if (print_od && enumerator.slaves()[i].coe_details) {
        std::cout << "CoE Object Dictionary:\n";
        MailboxHandler mbx(socket);
        CoEHandler coe(mbx);
        // We need non-const access to slave for mailbox ops (seq number update)
        // enumerate() populates const vector... actually vector is private in
        // Enumerator. enumerator.slaves() returns const ref. This is a problem
        // for CoEHandler which takes SlaveInfo& generally to update it (mbx
        // protocol stuff). Enumerator::slaves() returns const
        // std::vector<SlaveInfo>&. We cannot mutate the slave info? Wait,
        // SlaveInfo has mutable fields? (mbx_cnt?) Let's check SlaveInfo.hpp.
        // If not, we might need to cast away const or update Enumerator API.
        // Or maybe we can't update sequence count?

        // WORKAROUND: Cast const away for this sample, or fix API later.
        SlaveInfo &s = const_cast<SlaveInfo &>(enumerator.slaves()[i]);

        auto list_res = coe.read_od_list(s);
        if (list_res) {
          for (uint16_t idx : *list_res) {
            auto desc_res = coe.read_od_description(s, idx);
            if (desc_res) {
              std::cout << "  0x" << std::hex << idx << ": " << desc_res->name
                        << std::dec << "\n";
            } else {
              std::cout << "  0x" << std::hex << idx << ": (No desc)"
                        << std::dec << "\n";
            }
          }
        } else {
          std::cout << "  Failed to read OD list.\n";
        }
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
