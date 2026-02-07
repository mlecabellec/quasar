#pragma once

#include "resoem/common.hpp"
#include <optional>
#include <string>
#include <vector>

namespace resoem {

class RawSocket {
public:
  RawSocket(const std::string &interface_name);
  ~RawSocket();

  // Prevent copying
  RawSocket(const RawSocket &) = delete;
  RawSocket &operator=(const RawSocket &) = delete;

  // Allow moving
  RawSocket(RawSocket &&other) noexcept;
  RawSocket &operator=(RawSocket &&other) noexcept;

  // Send a raw frame. Returns number of bytes sent.
  size_t send(std::span<const byte> data);

  // Receive a raw frame. Returns number of bytes received.
  // Blocks until a frame is received or timeout (if implemented)
  // For now, simple blocking read.
  size_t receive(std::span<byte> buffer);

  // Set receive timeout in milliseconds
  void set_timeout(int timeout_ms);

  const std::string &interface_name() const { return interface_name_; }

  // Helper to get MAC address of the interface
  std::array<uint8_t, 6> get_mac_address() const;

private:
  std::string interface_name_;
  int sock_fd_ = -1;
  int if_index_ = -1;
};

} // namespace resoem
