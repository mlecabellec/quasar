#include "resoem/Enumerator.hpp"
#include "resoem/RawSocket.hpp"
#include <iostream>

using namespace resoem;

int main(int argc, char *argv[]) {
  // Step: Parse command line arguments
  std::cout << "Step: Parse command line arguments" << std::endl;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <interface_name>\n";
    return 1;
  }

  std::string ifname = argv[1];

  try {
    // Step: Initialize RawSocket
    std::cout << "Step: Initialize RawSocket for " << ifname << std::endl;
    RawSocket socket(ifname);

    // Step: Create Enumerator instance
    std::cout << "Step: Create Enumerator instance" << std::endl;
    Enumerator enumerator(socket);

    // Step: Perform slave enumeration
    std::cout << "Step: Perform slave enumeration (BRD count)" << std::endl;
    int count = enumerator.enumerate();

    // Assertion: Display number of slaves found
    std::cout << "Assertion: Enumeration complete. Slaves found: " << count
              << std::endl;

    // Step: Process Slaves and display information
    std::cout << "Step: Process Slaves and display information" << std::endl;
    const auto &slaves = enumerator.slaves();
    for (size_t i = 0; i < slaves.size(); ++i) {
      const auto &s = slaves[i];
      std::cout << "Slave " << (i + 1) << ": " << s.name << std::endl;
      std::cout << "  Vendor: 0x" << std::hex << s.vendor_id << " Product: 0x"
                << s.product_code << std::dec << std::endl;
      std::cout << "  Addr: 0x" << std::hex << s.configured_address << std::dec
                << " Parent: " << s.parent_index << std::endl;

      // Step: Enumerate SyncManagers for slave
      std::cout << "  Step: Enumerate SyncManagers for slave " << (i + 1)
                << std::endl;
      std::cout << "  SyncManagers: " << s.sync_managers.size() << std::endl;
      for (const auto &sm : s.sync_managers) {
        std::cout << "    SM" << (int)sm.type << ": Start=0x" << std::hex
                  << sm.start_addr << " Len=" << sm.length << std::dec
                  << std::endl;
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
