/**
 * @file slaveinfo.cpp
 * @brief EtherCAT Slave Information utility with expanded diagnostic reporting.
 */

#include "resoem/CoEHandler.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Diagnostics.hpp"
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace resoem;

void print_sm_type_desc(uint8_t type) {
    switch(type) {
        case 1: std::cout << "MbxOut (M->S)"; break;
        case 2: std::cout << "MbxIn  (S->M)"; break;
        case 3: std::cout << "Outputs (PDO)"; break;
        case 4: std::cout << "Inputs  (PDO)"; break;
        default: std::cout << "Undefined (" << (int)type << ")"; break;
    }
}

void print_slave_info(const SlaveInfo &s, int idx) {
  std::cout << "\n------------------------------------------------------" << std::endl;
  std::cout << "SLAVE #" << idx << " IDENTITY & STATUS" << std::endl;
  std::cout << "------------------------------------------------------" << std::endl;
  std::cout << "  Name:            " << s.name << "\n";
  std::cout << "  Configured Addr: 0x" << std::hex << s.configured_address << std::dec << "\n";
  std::cout << "  Vendor ID:       0x" << std::hex << s.vendor_id << std::dec << "\n";
  std::cout << "  Product Code:    0x" << std::hex << s.product_code << std::dec << "\n";
  std::cout << "  Current State:   0x" << std::hex << (s.current_state & 0x0F) 
            << " (" << (s.online ? "ONLINE" : "OFFLINE") << ")" << std::dec << "\n";
  
  if (s.mbx_out_length > 0) {
      std::cout << "  Mailbox:         Out: 0x" << std::hex << s.mbx_out_offset << " (" << std::dec << s.mbx_out_length << " B)"
                << " | In: 0x" << std::hex << s.mbx_in_offset << " (" << std::dec << s.mbx_in_length << " B)\n";
  }

  std::cout << "\n  [SYNC MANAGERS]" << std::endl;
  for (size_t i = 0; i < s.sync_managers.size(); ++i) {
    const auto &sm = s.sync_managers[i];
    std::cout << "    SM" << i << ": StartAddr=0x" << std::hex << std::setw(4) << std::setfill('0') << sm.start_addr
              << " | Len=" << std::dec << std::setw(4) << sm.length 
              << " | Flags=0x" << std::hex << std::setw(8) << sm.flags 
              << " | Type=";
    print_sm_type_desc(sm.type);
    std::cout << std::dec << "\n";
  }

  std::cout << "\n  [FMMU MAPPINGS]" << std::endl;
  if (s.fmmu.empty()) {
      std::cout << "    (None configured)\n";
  }
  for (size_t i = 0; i < s.fmmu.size(); ++i) {
    const auto &f = s.fmmu[i];
    std::cout << "    FMMU" << i << ": LogStart=0x" << std::hex << std::setw(8) << f.logical_start 
              << " | Len=" << std::dec << std::setw(3) << f.length 
              << " | Phys=0x" << std::hex << std::setw(4) << f.physical_start 
              << " | Type=" << (f.type == 1 ? "Read" : "Write") << std::dec << "\n";
  }

  std::cout << "\n  [PDO CONFIGURATION]" << std::endl;
  auto print_pdos = [](const std::string& label, const std::vector<PDOInfo>& pdos) {
      std::cout << "    " << label << " (" << pdos.size() << " PDOs):" << std::endl;
      for (const auto& p : pdos) {
          std::cout << "      - Index 0x" << std::hex << p.index << std::dec 
                    << " (" << p.entries.size() << " entries)" << std::endl;
          for (const auto& e : p.entries) {
              std::cout << "        - 0x" << std::hex << e.index << ":" << (int)e.subindex << std::dec 
                        << " | len=" << (int)e.bit_length << " bits" << std::endl;
          }
      }
  };
  print_pdos("RxPDOs (Master -> Slave)", s.rx_pdos);
  print_pdos("TxPDOs (Slave -> Master)", s.tx_pdos);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: slaveinfo IFNAME [-od]" << std::endl;
    std::cout << "Options:\n  -od   Attempt to dump CoE Object Dictionary for each slave" << std::endl;
    return 1;
  }

  std::string iface = argv[1];
  bool print_od = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-od") print_od = true;
  }

  try {
    std::cout << "======================================================" << std::endl;
    std::cout << "EtherCAT Slave Explorer (resoem)" << std::endl;
    std::cout << "======================================================" << std::endl;

    RawSocket socket(iface);
    Enumerator enumerator(socket);
    enumerator.set_verbose(true); // Always verbose for slaveinfo

    auto count_res = enumerator.enumerate();

    if (!count_res) {
      std::cerr << "[ERROR] Enumeration failed: " << count_res.error() << std::endl;
      return 1;
    }

    size_t count = *count_res;
    if (count == 0) {
      std::cout << "No slaves detected on " << iface << ".\n";
      return 0;
    }

    std::cout << "\nSuccess: Found " << count << " slaves. Mapping topology..." << std::endl;

    ProcessImage image;
    enumerator.configure_fmmu(image);
    
    // Attempt to move to PRE-OP to ensure mailbox config was valid
    std::cout << "Testing PRE-OP transition..." << std::endl;
    auto preop_res = enumerator.request_state_all(states::PRE_OP);
    
    enumerator.check_slaves_status();

    for (size_t i = 0; i < count; ++i) {
      const auto& s = enumerator.slaves()[i];
      print_slave_info(s, i);

      if (print_od && s.coe_details) {
        std::cout << "\n  [COE OBJECT DICTIONARY]" << std::endl;
        MailboxHandler mbx(socket);
        CoEHandler coe(mbx);
        
        // Non-const access to update mailbox counters
        SlaveInfo &mut_s = const_cast<SlaveInfo &>(s);

        auto list_res = coe.read_od_list(mut_s);
        if (list_res) {
          for (uint16_t idx : *list_res) {
            auto desc_res = coe.read_od_description(mut_s, idx);
            if (desc_res) {
              std::cout << "    0x" << std::hex << std::setw(4) << std::setfill('0') << idx 
                        << ": " << desc_res->name << std::dec << "\n";
            }
          }
        } else {
          std::cout << "    Failed to read OD list. (Mailbox might be busy or unsupported)\n";
        }
      }
    }

    std::cout << "\nExploration complete. Returning to INIT..." << std::endl;
    enumerator.request_state_all(states::INIT);

  } catch (const std::exception &e) {
    std::cerr << "[FATAL ERROR] " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
