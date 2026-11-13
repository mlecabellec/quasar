/**
 * @file common.cpp
 * @brief Common error formatting and safe little-endian encoding helpers for resoes.
 * @feature [TSK-20261113-001]
 */

#include "resoes/common.hpp"
#include "resoes/ResoesConstants.hpp"

namespace resoes {

/// @brief Format SlvError enum value cleanly.
std::string_view error_to_string(SlvError err) noexcept {
  switch (err) {
  case SlvError::Success:
    return "Success";
  case SlvError::HardwareIoFailed:
    return "Hardware I/O failed";
  case SlvError::InvalidParameter:
    return "Invalid parameter";
  case SlvError::BufferTooSmall:
    return "Buffer too small";
  case SlvError::OutOfBounds:
    return "Offset or index out of bounds";
  case SlvError::InvalidState:
    return "Invalid AL or server state";
  // Synchronization and loop safety errors
  case SlvError::MutexTimeout:
    return "Timed mutex acquisition timed out";
  case SlvError::LoopLimitExceeded:
    return "Safety iteration bound exceeded";
  // Mailbox and protocol errors
  case SlvError::MailboxFull:
    return "Mailbox buffer pool full";
  case SlvError::MailboxEmpty:
    return "Mailbox empty";
  case SlvError::ProtocolMismatch:
    return "Protocol header or toggle mismatch";
  case SlvError::ObjectNotFound:
    return "Object dictionary entry not found";
  case SlvError::AccessDenied:
    return "Object access denied";
  // Non-volatile & CoE abort codes
  case SlvError::EepromError:
    return "SII EEPROM operation failed";
  case SlvError::SdoAbort:
    return "CoE SDO transfer aborted";
  default:
    return "Unknown resoes error";
  }
}

/// @brief Read little-endian uint16_t from byte span safely.
Result<uint16_t> read_le16(std::span<const byte> src, size_t offset) noexcept {
  constexpr size_t WIDTH = 2U;
  // Verify bounds without integer wraparound.
  if ((offset > src.size()) || ((src.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  uint16_t lo = std::to_integer<uint16_t>(src[offset]);
  uint16_t hi = std::to_integer<uint16_t>(src[offset + 1U]);
  return static_cast<uint16_t>(lo | static_cast<uint16_t>(hi << 8U));
}

/// @brief Read little-endian uint32_t from byte span safely.
Result<uint32_t> read_le32(std::span<const byte> src, size_t offset) noexcept {
  constexpr size_t WIDTH = 4U;
  // Verify bounds without wraparound before reading 4 bytes.
  if ((offset > src.size()) || ((src.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  uint32_t b0 = std::to_integer<uint32_t>(src[offset]);
  uint32_t b1 = std::to_integer<uint32_t>(src[offset + 1U]);
  uint32_t b2 = std::to_integer<uint32_t>(src[offset + 2U]);
  uint32_t b3 = std::to_integer<uint32_t>(src[offset + 3U]);
  return b0 | (b1 << 8U) | (b2 << 16U) | (b3 << 24U);
}

/// @brief Read little-endian uint64_t from byte span safely.
Result<uint64_t> read_le64(std::span<const byte> src, size_t offset) noexcept {
  constexpr size_t WIDTH = 8U;
  // Verify bounds before unpacking 8 little-endian bytes.
  if ((offset > src.size()) || ((src.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  uint64_t accum = 0U;
  for (size_t idx = 0U; idx < WIDTH; ++idx) {
    uint64_t val = std::to_integer<uint64_t>(src[offset + idx]);
    accum |= (val << (idx * constants::BITS_PER_BYTE));
  }
  return accum;
}

/// @brief Write little-endian uint16_t into byte span safely.
Result<> write_le16(std::span<byte> dst, size_t offset,
                    uint16_t value) noexcept {
  constexpr size_t WIDTH = 2U;
  if ((offset > dst.size()) || ((dst.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  dst[offset] = static_cast<byte>(value & 0xFFU);
  dst[offset + 1U] = static_cast<byte>((value >> 8U) & 0xFFU);
  return {};
}

/// @brief Write little-endian uint32_t into byte span safely.
Result<> write_le32(std::span<byte> dst, size_t offset,
                    uint32_t value) noexcept {
  constexpr size_t WIDTH = 4U;
  if ((offset > dst.size()) || ((dst.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  for (size_t idx = 0U; idx < WIDTH; ++idx) {
    dst[offset + idx] =
        static_cast<byte>((value >> (idx * constants::BITS_PER_BYTE)) & 0xFFU);
  }
  return {};
}

/// @brief Write little-endian uint64_t into byte span safely.
Result<> write_le64(std::span<byte> dst, size_t offset,
                    uint64_t value) noexcept {
  constexpr size_t WIDTH = 8U;
  if ((offset > dst.size()) || ((dst.size() - offset) < WIDTH)) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  for (size_t idx = 0U; idx < WIDTH; ++idx) {
    dst[offset + idx] =
        static_cast<byte>((value >> (idx * constants::BITS_PER_BYTE)) & 0xFFU);
  }
  return {};
}

} // namespace resoes
