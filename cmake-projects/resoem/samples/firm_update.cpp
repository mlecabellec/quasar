/**
 * @file firm_update.cpp
 * @brief EtherCAT Firmware Update utility using FoE (File over EtherCAT).
 * @details Demonstrates the firmware update workflow:
 * 1. Switch slave to BOOT state.
 * 2. Send firmware file via FoE Write Request.
 * 3. Wait for ACKs and monitor progress.
 * 4. Return slave to INIT for reboot.
 */

#include "resoem/Enumerator.hpp"
#include "resoem/FoEHandler.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Diagnostics.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

using namespace resoem;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cout << "Usage: firm_update IFNAME SLAVE FILENAME [PASSWORD] [-v]\n";
    std::cout << "Example: firm_update eth0 1 firmware.bin 0x00000000 -v\n";
    return 1;
  }

  std::string iface = argv[1];
  int slave_pos = std::stoi(argv[2]);
  std::string filename = argv[3];
  uint32_t password = 0;
  bool verbose = false;

  for (int i = 4; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-v") verbose = true;
      else if (arg.starts_with("0x")) password = static_cast<uint32_t>(std::stoul(arg, nullptr, 16));
  }

  try {
    std::cout << "======================================================" << std::endl;
    std::cout << "EtherCAT Firmware Updater (Resoem)" << std::endl;
    std::cout << "======================================================" << std::endl;

    std::cout << "[STEP 1] Initializing interface " << iface << "..." << std::endl;
    RawSocket socket(iface);
    Enumerator enumerator(socket);
    enumerator.set_verbose(verbose);

    if (Result<size_t> res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "[ERROR] No slaves found on the bus.\n";
      return 1;
    }

    if (slave_pos < 1 || slave_pos > (int)enumerator.slaves().size()) {
      std::cout << "[ERROR] Slave " << slave_pos << " not found.\n";
      return 1;
    }

    int slave_idx = slave_pos - 1;
    // We need a mutable reference to track mailbox sequence numbers
    SlaveInfo &slave = const_cast<SlaveInfo &>(enumerator.slaves()[slave_idx]);

    std::cout << "[STEP 2] Preparing slave " << slave_pos << " (" << slave.name << ")..." << std::endl;
    
    // Check if FoE is supported
    if (!(slave.mbx_protocols & 0x08)) {
        std::cout << "[WARNING] Slave SII does not claim FoE support. Attempting anyway...\n";
    }

    std::cout << "[STEP 3] Requesting BOOT state..." << std::endl;
    Result<uint16_t> boot_res = enumerator.request_state(static_cast<uint16_t>(slave_idx), states::BOOT);
    if (!boot_res) {
      std::cout << "[WARNING] Failed to switch to BOOT state: " << static_cast<int>(boot_res.error()) << "\n";
      std::cout << "          Continuing anyway (some slaves support FoE in PRE-OP).\n";
    } else {
        std::cout << "[INFO] Slave reached BOOT state.\n";
    }

    // Read firmware file into memory
    std::cout << "[STEP 4] Loading firmware file: " << filename << "..." << std::endl;
    std::ifstream infile(filename, std::ios::binary | std::ios::ate);
    if (!infile) {
      std::cout << "[ERROR] Failed to open file: " << filename << "\n";
      return 1;
    }
    size_t size = infile.tellg();
    infile.seekg(0);
    std::vector<byte> fw_data(size);
    infile.read(reinterpret_cast<char *>(fw_data.data()), static_cast<std::streamsize>(size));
    std::cout << "[INFO] Firmware loaded (" << size << " bytes).\n";

    std::cout << "[STEP 5] Starting FoE Download..." << std::endl;
    MailboxHandler mbx(socket);
    mbx.set_verbose(verbose);
    FoEHandler foe(mbx);

    // Extract filename from path for the FoE request
    std::string name_only = filename;
    size_t last_slash = filename.find_last_of("\\/");
    if (last_slash != std::string::npos)
      name_only = filename.substr(last_slash + 1);

    std::cout << "  - Destination filename: " << name_only << std::endl;
    std::cout << "  - Transferring... " << std::flush;

    Result<> res = foe.write_file(slave, name_only, password, fw_data);
    
    if (res) {
      std::cout << "\n[SUCCESS] Firmware download completed successfully." << std::endl;
    } else {
      std::cout << "\n[ERROR] Firmware download failed: " << static_cast<int>(res.error()) << std::endl;
      
      // Attempt to read AL Status Code for more info
      int wkc;
      uint16_t code = enumerator.read_register_fprd<uint16_t>(slave.configured_address, regs::AL_STATUS_CODE, wkc);
      if (wkc > 0 && code != 0) {
          std::cerr << "        AL Status: 0x" << std::hex << code << ": " << al_status_code_to_string(code) << std::dec << std::endl;
      }
      return 1;
    }

    std::cout << "[STEP 6] Resetting slave to INIT..." << std::endl;
    enumerator.request_state(static_cast<uint16_t>(slave_idx), states::INIT);
    
    std::cout << "[INFO] Waiting for slave to reboot... " << std::flush;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Done." << std::endl;

    std::cout << "======================================================" << std::endl;
    std::cout << "Update Complete." << std::endl;
    std::cout << "======================================================" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "[FATAL] " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
