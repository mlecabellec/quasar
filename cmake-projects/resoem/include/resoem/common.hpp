#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <stdexcept>
#include <vector>

namespace resoem {

using byte = uint8_t;

enum class ECError {
  Success = 0,
  Timeout,
  MailboxError,
  ProtocolError,
  InvalidResponse,
  SlaveOffline,
  SDOAbort,
  FoEError,
  EoEError
};

// Result type for C++23 style error handling
template <typename T = void>
using Result = std::expected<T, ECError>;

// EtherCAT constants
constexpr uint16_t ETHERCAT_ETHERTYPE = 0x88A4;
constexpr size_t ETHERNET_HEADER_SIZE = 14;
constexpr size_t ETHERCAT_HEADER_SIZE = 2;
constexpr size_t MIN_FRAME_SIZE = 64; // Minimum Ethernet frame size

// Exception classes
class SocketError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class FrameError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace resoem
