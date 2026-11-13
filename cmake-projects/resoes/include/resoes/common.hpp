/**
 * @file common.hpp
 * @brief Common error types, Result alias, and safe little-endian span helpers for resoes.
 * @feature [TSK-20261113-001] resoes EtherCAT SubDevice Stack Initialization
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace resoes {

/// @brief Byte alias matching Quasar raw-buffer conventions.
using byte = std::byte;

/**
 * @brief Error codes returned by resoes SubDevice stack operations.
 * @feature [TSK-20261113-001]
 */
enum class SlvError : uint8_t {
  Success = 0,          ///< Operation completed without error
  HardwareIoFailed,     ///< ESC hardware read or write failed
  InvalidParameter,     ///< Invalid argument or null dependency
  BufferTooSmall,       ///< Provided span buffer is smaller than required
  OutOfBounds,          ///< Register, offset, or object index out of range
  InvalidState,         ///< Operation rejected in current AL or server state
  MutexTimeout,         ///< Failed to acquire timed_mutex within bound
  LoopLimitExceeded,    ///< Hard safety iteration bound reached [CS-0010.37]
  MailboxFull,          ///< Mailbox buffer pool exhausted
  MailboxEmpty,         ///< No mailbox message pending
  ProtocolMismatch,     ///< Unexpected mailbox or SDO header/toggle
  ObjectNotFound,       ///< Object dictionary index/subindex missing
  AccessDenied,         ///< Object dictionary read/write permission denied
  // Non-volatile & CoE abort codes
  EepromError,          ///< SII EEPROM backing read/write/reload failed
  SdoAbort              ///< CoE SDO transfer aborted (details in abort code)
};

/**
 * @brief Result wrapper propagating SlvError values cleanly without raw exceptions.
 * @feature [TSK-20261113-001] [CS-0020.48]
 */
template <typename T = void> using Result = std::expected<T, SlvError>;

/**
 * @brief Convert SlvError enum value to a static human-readable description.
 * @param err Error value to format.
 * @return Null-terminated static string view describing the error.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] std::string_view error_to_string(SlvError err) noexcept;

/**
 * @brief Decode an unsigned 16-bit little-endian value from a byte span.
 * @param src Source byte span.
 * @param offset Byte offset inside the span.
 * @return Decoded uint16_t or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<uint16_t> read_le16(std::span<const byte> src,
                                         size_t offset) noexcept;

/**
 * @brief Decode an unsigned 32-bit little-endian value from a byte span.
 * @param src Source byte span.
 * @param offset Byte offset inside the span.
 * @return Decoded uint32_t or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<uint32_t> read_le32(std::span<const byte> src,
                                         size_t offset) noexcept;

/**
 * @brief Decode an unsigned 64-bit little-endian value from a byte span.
 * @param src Source byte span.
 * @param offset Byte offset inside the span.
 * @return Decoded uint64_t or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<uint64_t> read_le64(std::span<const byte> src,
                                         size_t offset) noexcept;

/**
 * @brief Encode an unsigned 16-bit integer in little-endian order into a byte span.
 * @param dst Destination byte span.
 * @param offset Byte offset inside the span.
 * @param value Value to encode.
 * @return Success or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<> write_le16(std::span<byte> dst, size_t offset,
                                  uint16_t value) noexcept;

/**
 * @brief Encode an unsigned 32-bit integer in little-endian order into a byte span.
 * @param dst Destination byte span.
 * @param offset Byte offset inside the span.
 * @param value Value to encode.
 * @return Success or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<> write_le32(std::span<byte> dst, size_t offset,
                                  uint32_t value) noexcept;

/**
 * @brief Encode an unsigned 64-bit integer in little-endian order into a byte span.
 * @param dst Destination byte span.
 * @param offset Byte offset inside the span.
 * @param value Value to encode.
 * @return Success or SlvError::BufferTooSmall on bounds failure.
 * @feature [TSK-20261113-001]
 */
[[nodiscard]] Result<> write_le64(std::span<byte> dst, size_t offset,
                                  uint64_t value) noexcept;

} // namespace resoes
