#include "resoem/CoEHandler.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/RawSocket.hpp"
#include <iostream>

using namespace resoem;

int main(int argc, char *argv[]) {
  // Step: Parse command line arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <interface_name>\n";
    return 1;
  }

  std::string ifname = argv[1];

  try {
    // Step: Initialize RawSocket
    RawSocket socket(ifname);

    // Step: Enumerate slaves to get addresses and mailbox configs
    Enumerator enumerator(socket);
    int count = enumerator.enumerate();
    std::cout << "Found " << count << " slaves\n";

    if (count == 0) {
      std::cout << "No slaves found. Exiting.\n";
      return 0;
    }

    // Step: Initialize Mailbox and CoE handlers
    MailboxHandler mailbox(socket);
    CoEHandler coe(mailbox);

    const auto &slaves = enumerator.slaves();
    for (size_t i = 0; i < slaves.size(); ++i) {
      // Need mutable reference to update mailbox counter
      auto &slave = const_cast<SlaveInfo &>(slaves[i]);

      std::cout << "Processing Slave " << (i + 1) << ": " << slave.name << "\n";

      if (slave.mbx_in_length == 0 || slave.mbx_out_length == 0) {
        std::cout << "  Slave does not support mailbox communication.\n";
        continue;
      }

      // Step: Attempt SDO Read of Index 0x1008 (Device Name)
      byte name_buf[64];
      size_t actual_size = 0;
      std::cout << "  Reading SDO 0x1008:00 (Device Name)...\n";
      auto err = coe.sdo_read(slave, 0x1008, 0x00, name_buf, actual_size);

      if (err == CoEError::Success) {
        std::string name(reinterpret_cast<char *>(name_buf), actual_size);
        std::cout << "  Assertion: CoE Read Success. Value: \"" << name
                  << "\"\n";
      } else {
        std::cerr << "  Assertion Failed: CoE Read returned error " << (int)err
                  << "\n";
      }

      // Step: Attempt SDO Read of Index 0x1018:01 (Vendor ID)
      uint32_t vendor_id = 0;
      std::cout << "  Reading SDO 0x1018:01 (Vendor ID)...\n";
      err =
          coe.sdo_read(slave, 0x1018, 0x01,
                       std::span<byte>(reinterpret_cast<byte *>(&vendor_id), 4),
                       actual_size);

      if (err == CoEError::Success && actual_size == 4) {
        std::cout << "  Assertion: CoE Read Success. Vendor ID: 0x" << std::hex
                  << vendor_id << std::dec << "\n";
      } else {
        std::cerr << "  Assertion Failed: CoE Read returned error " << (int)err
                  << "\n";
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "Error during CoE verification: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
