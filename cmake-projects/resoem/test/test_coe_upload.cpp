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
    Result<size_t> count_result = enumerator.enumerate();
    if (!count_result.has_value()) {
      std::cerr << "Error during enumeration: " << static_cast<int>(count_result.error()) << std::endl;
      return 1;
    }
    size_t count = count_result.value();
    std::cout << "Found " << count << " slaves\n";

    if (count == 0) {
      std::cout << "No slaves found. Exiting.\n";
      return 0;
    }

    // Step: Initialize Mailbox and CoE handlers
    MailboxHandler mailbox(socket);
    CoEHandler coe(mailbox);

    const std::vector<SlaveInfo> &slaves = enumerator.slaves();
    for (size_t i = 0; i < slaves.size(); ++i) {
      // Need mutable reference to update mailbox counter
      SlaveInfo &slave = const_cast<SlaveInfo &>(slaves[i]);

      std::cout << "Processing Slave " << (i + 1) << ": " << slave.name << "\n";

      if (slave.mbx_in_length == 0 || slave.mbx_out_length == 0) {
        std::cout << "  Slave does not support mailbox communication.\n";
        continue;
      }

      // Step: Attempt SDO Read of Index 0x1008 (Device Name)
      // [Compliance Proof] FE-0040.4.1.1: Support SDO Read for expedited and normal transfers.
      byte name_buf[64];
      std::cout << "  Reading SDO 0x1008:00 (Device Name)...\n";
      Result<size_t> err = coe.sdo_read(slave, 0x1008, 0x00, name_buf);

      if (err) {
        std::string name(reinterpret_cast<char *>(name_buf), *err);
        std::cout << "  Assertion: CoE Read Success. Value: \"" << name
                  << "\"\n";
      } else {
        std::cerr << "  Assertion Failed: CoE Read returned error " << static_cast<int>(err.error())
                  << "\n";
      }

      // Step: Attempt SDO Read of Index 0x1018:01 (Vendor ID)
      uint32_t vendor_id = 0;
      std::cout << "  Reading SDO 0x1018:01 (Vendor ID)...\n";
      err =
          coe.sdo_read(slave, 0x1018, 0x01,
                       std::span<byte>(reinterpret_cast<byte *>(&vendor_id), 4));

      if (err && *err == 4) {
        std::cout << "  Assertion: CoE Read Success. Vendor ID: 0x" << std::hex
                  << vendor_id << std::dec << "\n";
      } else {
        std::cerr << "  Assertion Failed: CoE Read returned error " << (err ? 0 : static_cast<int>(err.error()))
                  << "\n";
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "Error during CoE verification: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
