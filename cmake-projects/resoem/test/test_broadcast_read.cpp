#include "resoem/EtherCATFrame.hpp"
#include "resoem/RawSocket.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace resoem;

void print_buffer(const std::span<const byte> buffer) {
  std::cout << std::hex << std::setfill('0');
  for (size_t i = 0; i < buffer.size(); ++i) {
    std::cout << std::setw(2) << static_cast<int>(buffer[i]) << " ";
    if ((i + 1) % 16 == 0)
      std::cout << "\n";
  }
  std::cout << std::dec << "\n";
}

int main(int argc, char *argv[]) {
  // Step: Parse command line arguments
  std::cout << "Step: Parse command line arguments" << std::endl;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <interface_name>\n";
    return 1;
  }

  std::string ifname = argv[1];

  try {
    // Step: Open interface
    std::cout << "Step: Opening interface " << ifname << "..." << std::endl;
    RawSocket socket(ifname);

    // Step: Retrieve and display MAC address
    std::cout << "Step: Retrieve and display MAC address" << std::endl;
    auto mac = socket.get_mac_address();
    std::cout << "Interface opened. MAC: ";
    for (int i = 0; i < 6; ++i)
      std::cout << std::hex << (int)mac[i] << (i < 5 ? ":" : "\n") << std::dec;

    // Step: Initialize FrameBuilder
    std::cout << "Step: Initialize FrameBuilder" << std::endl;
    FrameBuilder builder;

    // Step: Configure Broadcast Read (BRD) datagram
    // BRD command = 0x07, ARP = 0x0000, ADO = 0x0000
    std::cout
        << "Step: Configure Broadcast Read (BRD) datagram for register 0x0000"
        << std::endl;
    uint8_t data_placeholder[2] = {0, 0};
    builder.add_datagram(0x07, 0x01, 0x0000, 0x0000,
                         std::span<const byte>(data_placeholder, 2));

    // Step: Build frame
    std::cout << "Step: Build frame" << std::endl;
    auto frame = builder.build();

    // Step: Send frame
    std::cout << "Step: Sending frame (" << frame.size() << " bytes)..."
              << std::endl;
    print_buffer(frame);
    socket.send(frame);

    // Step: Wait for response
    std::cout << "Step: Waiting for response..." << std::endl;
    std::vector<byte> receive_buffer(1500);
    size_t received = socket.receive(receive_buffer);

    // Assertion: Check if data was received
    std::cout << "Assertion: Check if data was received" << std::endl;
    if (received > 0) {
      std::cout << "Received " << received << " bytes." << std::endl;
      print_buffer(std::span<const byte>(receive_buffer.data(), received));

      // Step: Extract Working Counter (WKC)
      std::cout << "Step: Extract Working Counter (WKC)" << std::endl;
      if (received > ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + 10 + 2 + 2) {
        uint16_t wkc;
        std::memcpy(&wkc, receive_buffer.data() + 28, 2);
        std::cout << "Assertion: Working Counter (WKC) is: " << wkc
                  << std::endl;
      }
    } else {
      std::cout << "Timeout or no data." << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
