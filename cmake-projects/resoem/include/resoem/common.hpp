/**
 * @file common.hpp
 * @brief Common types, constants, and error handling for the resoem library.
 */

#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <stdexcept>
#include <vector>

namespace resoem {

/**
 * @brief Basic byte type.
 */
using byte = uint8_t;

/**
 * @brief Error codes for EtherCAT operations.
 */
enum class ECError {
  Success = 0,      ///< Operation completed successfully
  Timeout,          ///< Operation timed out
  MailboxError,     ///< Error in mailbox communication
  ProtocolError,    ///< Protocol-level error
  InvalidResponse,  ///< Received an unexpected or invalid response
  SlaveOffline,     ///< Target slave is offline
  SDOAbort,         ///< SDO transfer was aborted by the slave
  FoEError,         ///< File over EtherCAT error
  EoEError          ///< Ethernet over EtherCAT error
};

/**
 * @brief Result type for C++23 style error handling.
 * @tparam T The type of the successful result value.
 */
template <typename T = void>
using Result = std::expected<T, ECError>;

// EtherCAT constants
constexpr uint16_t ETHERCAT_ETHERTYPE = 0x88A4; ///< EtherCAT Ethernet type (0x88A4)
constexpr size_t ETHERNET_HEADER_SIZE = 14;     ///< Standard Ethernet header size
constexpr size_t ETHERCAT_HEADER_SIZE = 2;       ///< EtherCAT frame header size
constexpr size_t MIN_FRAME_SIZE = 64;           ///< Minimum Ethernet frame size

/**
 * @brief Exception thrown for socket-related errors.
 */
class SocketError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

/**
 * @brief Exception thrown for EtherCAT frame-related errors.
 */
class FrameError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace resoem
