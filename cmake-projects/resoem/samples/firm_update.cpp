/**
 * @file firm_update.cpp
 * @brief Port of firm_update.c to resoem
 */

#include "resoem/Enumerator.hpp"
#include "resoem/FoEHandler.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/RawSocket.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace resoem;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cout << "Usage: firm_update IFNAME SLAVE FILENAME [PASSWORD]\n";
    return 1;
  }

  std::string iface = argv[1];
  int slave_pos = std::stoi(argv[2]);
  std::string filename = argv[3];
  uint32_t password = 0;
  if (argc > 4)
    password = std::stoul(argv[4], nullptr, 16);

  try {
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "No slaves found.\n";
      return 1;
    }

    if (slave_pos < 1 || slave_pos > (int)enumerator.slaves().size()) {
      std::cout << "Slave " << slave_pos << " not found.\n";
      return 1;
    }

    int slave_idx = slave_pos - 1;
    SlaveInfo &slave = const_cast<SlaveInfo &>(enumerator.slaves()[slave_idx]);

    std::cout << "Requesting BOOT state for slave " << slave_pos << "...\n";
    if (!enumerator.request_state(slave_idx, states::BOOT)) {
      std::cout << "Failed to switch to BOOT state. Attempting to continue "
                   "anyway (some slaves support FoE in Init/PreOp/SafeOp).\n";
    }

    // Read firmware file
    std::ifstream infile(filename, std::ios::binary | std::ios::ate);
    if (!infile) {
      std::cout << "Failed to open file: " << filename << "\n";
      return 1;
    }
    size_t size = infile.tellg();
    infile.seekg(0);
    std::vector<byte> fw_data(size);
    infile.read(reinterpret_cast<char *>(fw_data.data()), size);

    std::cout << "Downloading firmware (" << size << " bytes) to " << slave.name
              << "...\n";

    MailboxHandler mbx(socket);
    FoEHandler foe(mbx);

    // Extract filename from path for the FoE request
    std::string name_only = filename;
    size_t last_slash = filename.find_last_of("\\/");
    if (last_slash != std::string::npos)
      name_only = filename.substr(last_slash + 1);

    auto res = foe.write_file(slave, name_only, password, fw_data);
    if (res) {
      std::cout << "Firmware update successful.\n";
    } else {
      std::cout << "Firmware update failed.\n";
      return 1;
    }

    std::cout << "Requesting INIT state...\n";
    enumerator.request_state(slave_idx, states::INIT);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
