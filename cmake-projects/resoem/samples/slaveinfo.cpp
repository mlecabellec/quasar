/**
 * @file slaveinfo.cpp
 * @brief EtherCAT Slave Information utility with expanded diagnostic reporting.
 * @details Provides a detailed report of all slaves on the bus, including identity,
 * state, mailbox configuration, SyncManagers, FMMUs, and PDO mappings.
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
#include <vector>

using namespace resoem;

/**
 * @brief Helper to get a human-readable description of SyncManager types.
 */
void print_sm_type_desc(uint8_t type) {
    switch(type) {
        case 1: std::cout << "MbxOut (Master -> Slave)"; break;
        case 2: std::cout << "MbxIn  (Slave -> Master)"; break;
        case 3: std::cout << "Outputs (Process Data Out)"; break;
        case 4: std::cout << "Inputs  (Process Data In)"; break;
        default: std::cout << "Undefined (" << (int)type << ")"; break;
    }
}

/**
 * @brief Helper to get AL State names.
 */
std::string get_state_name(uint16_t state) {
    switch(state & al_status::STATE_MASK) {
        case states::INIT:    return "INIT";
        case states::PRE_OP:  return "PRE-OP";
        case states::BOOT:    return "BOOT";
        case states::SAFE_OP: return "SAFE-OP";
        case states::OP:      return "OPERATIONAL";
        default:              return "UNKNOWN";
    }
}

/**
 * @brief Prints comprehensive information about a single slave.
 */
void print_slave_info(const SlaveInfo &s, int idx) {
  std::cout << "\n======================================================" << std::endl;
  std::cout << "SLAVE #" << idx << ": " << (s.name.empty() ? "(Unnamed Device)" : s.name) << std::endl;
  std::cout << "======================================================" << std::endl;
  
  std::cout << " [IDENTITY]" << std::endl;
  std::cout << "  Vendor ID:       0x" << std::hex << std::setw(8) << std::setfill('0') << s.vendor_id << std::dec << "\n";
  std::cout << "  Product Code:    0x" << std::hex << std::setw(8) << std::setfill('0') << s.product_code << std::dec << "\n";
  std::cout << "  Revision No:     0x" << std::hex << std::setw(8) << std::setfill('0') << s.revision_number << std::dec << "\n";
  std::cout << "  Serial No:       0x" << std::hex << std::setw(8) << std::setfill('0') << s.serial_number << std::dec << "\n";

  std::cout << "\n [NETWORK]" << std::endl;
  std::cout << "  Station Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << s.configured_address << std::dec << "\n";
  std::cout << "  Alias Address:   0x" << std::hex << std::setw(4) << std::setfill('0') << s.alias_address << std::dec << "\n";
  std::cout << "  Current State:   " << get_state_name(s.current_state) 
            << " (0x" << std::hex << s.current_state << std::dec << ")" 
            << " | " << (s.online ? "ONLINE" : "OFFLINE") << "\n";
  
  if (s.mbx_out_length > 0) {
      std::cout << "  Mailbox Config:  Out: Offs=0x" << std::hex << s.mbx_out_offset << " Len=" << std::dec << s.mbx_out_length << " B"
                << " | In: Offs=0x" << std::hex << s.mbx_in_offset << " Len=" << std::dec << s.mbx_in_length << " B\n";
      std::cout << "  Protocols:       ";
      if (s.mbx_protocols & 0x01) std::cout << "AoE ";
      if (s.mbx_protocols & 0x02) std::cout << "EoE ";
      if (s.mbx_protocols & 0x04) std::cout << "CoE ";
      if (s.mbx_protocols & 0x08) std::cout << "FoE ";
      if (s.mbx_protocols & 0x10) std::cout << "SoE ";
      if (s.mbx_protocols & 0x20) std::cout << "VoE ";
      std::cout << "\n";
  }

  std::cout << "\n [TOPOLOGY]" << std::endl;
  std::cout << "  Parent Index:    " << s.parent_index << "\n";
  for (int p = 0; p < 4; ++p) {
      std::cout << "  Port " << p << ":          " << (s.ports[p].active ? "LINK UP  " : "LINK DOWN")
                << " | Loop: " << (s.ports[p].loop_closed ? "CLOSED" : "OPEN  ");
      if (s.ports[p].neighbor_idx != -1) {
          std::cout << " | Neighbor: Slave " << s.ports[p].neighbor_idx;
      }
      std::cout << "\n";
  }

  std::cout << "\n [SYNC MANAGERS]" << std::endl;
  if (s.sync_managers.empty()) {
      std::cout << "  (No SyncManagers found in SII)\n";
  }
  for (size_t i = 0; i < s.sync_managers.size(); ++i) {
    const auto &sm = s.sync_managers[i];
    std::cout << "  SM" << i << ": Start=0x" << std::hex << std::setw(4) << std::setfill('0') << sm.start_addr
              << " | Len=" << std::dec << std::setw(4) << sm.length 
              << " | Flags=0x" << std::hex << std::setw(8) << sm.flags 
              << " | ";
    print_sm_type_desc(sm.type);
    std::cout << std::dec << "\n";
  }

  std::cout << "\n [FMMU MAPPINGS]" << std::endl;
  if (s.fmmu.empty()) {
      std::cout << "  (None configured yet - FMMUs are set during transition to SAFE-OP)\n";
  } else {
      for (size_t i = 0; i < s.fmmu.size(); ++i) {
        const auto &f = s.fmmu[i];
        std::cout << "  FMMU" << i << ": LogStart=0x" << std::hex << std::setw(8) << f.logical_start 
                  << " | Len=" << std::dec << std::setw(3) << f.length 
                  << " | Phys=0x" << std::hex << std::setw(4) << f.physical_start 
                  << " | Type=" << (f.type == 1 ? "Read (Inputs)" : "Write (Outputs)") << std::dec << "\n";
      }
  }

  std::cout << "\n [PDO CONFIGURATION]" << std::endl;
  auto print_pdos = [](const std::string& label, const std::vector<PDOInfo>& pdos) {
      std::cout << "  " << label << " (" << pdos.size() << " PDOs):" << std::endl;
      if (pdos.empty()) std::cout << "    (None)\n";
      for (const auto& p : pdos) {
          std::cout << "    - PDO Index 0x" << std::hex << p.index << std::dec 
                    << " (" << p.entries.size() << " entries)" << std::endl;
          for (const auto& e : p.entries) {
              std::cout << "      - 0x" << std::hex << std::setw(4) << std::setfill('0') << e.index 
                        << ":" << std::setw(2) << (int)e.subindex << std::dec 
                        << " | Bits=" << std::setw(2) << (int)e.bit_length 
                        << " | " << (e.name.empty() ? "(Unnamed)" : e.name) << std::endl;
          }
      }
  };
  print_pdos("RxPDOs (Master -> Slave / Outputs)", s.rx_pdos);
  print_pdos("TxPDOs (Slave -> Master / Inputs )", s.tx_pdos);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: slaveinfo IFNAME [-od] [-v]\n";
    std::cout << "Options:\n";
    std::cout << "  -od   Attempt to dump CoE Object Dictionary for each slave\n";
    std::cout << "  -v    Enable verbose internal library logging\n";
    return 1;
  }

  std::string iface = argv[1];
  bool print_od = false;
  bool verbose = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-od") print_od = true;
    if (arg == "-v") verbose = true;
  }

  try {
    std::cout << "======================================================" << std::endl;
    std::cout << "EtherCAT Slave Explorer (Resoem)" << std::endl;
    std::cout << "======================================================" << std::endl;

    std::cout << "[PHASE 1] Initializing Network on " << iface << "..." << std::endl;
    RawSocket socket(iface);
    Enumerator enumerator(socket);
    enumerator.set_verbose(verbose); 

    std::cout << "[PHASE 2] Enumerating Slaves..." << std::endl;
    auto count_res = enumerator.enumerate();

    if (!count_res) {
      std::cerr << "[ERROR] Enumeration failed: " << static_cast<int>(count_res.error()) << std::endl;
      return 1;
    }

    size_t count = *count_res;
    if (count == 0) {
      std::cout << "[INFO] No slaves detected on " << iface << ". Check connection and cabling." << std::endl;
      return 0;
    }

    std::cout << "[SUCCESS] Found " << count << " slaves. Performing detailed analysis..." << std::endl;

    // Transition to PRE-OP to enable mailbox communication if needed for OD browsing
    std::cout << "[PHASE 3] Testing transition to PRE-OP..." << std::endl;
    auto preop_res = enumerator.request_state_all(states::PRE_OP);
    if (!preop_res) {
        std::cerr << "[WARNING] Failed to reach PRE-OP state globally. Mailbox operations might fail." << std::endl;
    }
    
    // Update status to get latest AL states
    enumerator.check_slaves_status();

    for (size_t i = 0; i < count; ++i) {
      const auto& s = enumerator.slaves()[i];
      print_slave_info(s, i);

      // [OPTIONAL] CoE Object Dictionary browsing
      if (print_od) {
        if (!(s.mbx_protocols & 0x04)) {
            std::cout << "\n  [COE OBJECT DICTIONARY]\n    Skipped: Slave does not support CoE.\n";
        } else {
            std::cout << "\n  [COE OBJECT DICTIONARY]" << std::endl;
            MailboxHandler mbx(socket);
            mbx.set_verbose(verbose);
            CoEHandler coe(mbx);
            coe.set_verbose(verbose);            
            // Need mutable reference to update mailbox counters internally
            SlaveInfo &mut_s = const_cast<SlaveInfo &>(s);

            std::cout << "    Reading OD list... " << std::flush;
            auto list_res = coe.read_od_list(mut_s);
            if (list_res) {
              std::cout << "Found " << list_res->size() << " objects." << std::endl;
              for (uint16_t idx : *list_res) {
                auto desc_res = coe.read_od_description(mut_s, idx);
                if (desc_res) {
                  std::cout << "    - 0x" << std::hex << std::setw(4) << std::setfill('0') << idx 
                            << ": " << desc_res->name 
                            << " (Type=0x" << desc_res->datatype 
                            << ", MaxSub=" << (int)desc_res->max_subindex << ")" 
                            << std::dec << "\n";
                }
              }
            } else {
              std::cout << "FAILED. (Mailbox might be busy, in error state, or timeout)\n";
              int wkc;
              uint16_t al_code = read_register_fprd<uint16_t>(socket, s.configured_address, regs::AL_STATUS_CODE, wkc);
              if (wkc > 0 && al_code != 0) {
                  std::cout << "    AL Status Code: 0x" << std::hex << al_code << ": " 
                            << al_status_code_to_string(al_code) << std::dec << "\n";
              }
            }
        }
      }
    }

    std::cout << "\n[PHASE 4] Cleaning up..." << std::endl;
    std::cout << "Returning all slaves to INIT state..." << std::endl;
    enumerator.request_state_all(states::INIT);

    std::cout << "======================================================" << std::endl;
    std::cout << "Exploration Finished Successfully." << std::endl;
    std::cout << "======================================================" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "[FATAL ERROR] An unexpected exception occurred: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

// Minimal helper for AL Status Code read if CoE fails
template <typename T>
T read_register_fprd(RawSocket &socket, uint16_t configured_addr, uint16_t reg, int &wkc) {
  T val{};
  std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  
  FrameBuilder builder;
  builder.add_datagram(cmds::FPRD, 0, configured_addr, reg, buf);
  auto frame = builder.build();
  socket.send(frame);
  
  std::vector<byte> rx(1500);
  size_t rec = socket.receive(rx);
  if (rec >= 28 + sizeof(T)) {
      std::memcpy(&val, rx.data() + 26, sizeof(T));
      std::memcpy(&wkc, rx.data() + 26 + sizeof(T), 2);
  } else {
      wkc = 0;
  }
  return val;
}
