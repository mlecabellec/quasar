/**
 * @file IEepromStorage.hpp
 * @brief Abstract non-volatile SII EEPROM backing store interface for ESC emulation.
 * @feature [TSK-20261113-001] Symmetrical to SOES esc_eep HAL
 */

#pragma once

#include "resoes/common.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace resoes {

/**
 * @brief Abstract SII EEPROM backing store (RAM, flash, or file backed).
 * @feature [TSK-20261113-001]
 */
class IEepromStorage {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~IEepromStorage() noexcept = default;

  /**
   * @brief Return total backing store capacity in bytes.
   * @return EEPROM size in bytes.
   */
  [[nodiscard]] virtual size_t size_bytes() const noexcept = 0;

  /**
   * @brief Read SII bytes starting at byte offset.
   * @param byte_offset Byte offset inside EEPROM.
   * @param buffer Destination buffer slice (`4` or `8` bytes typical).
   * @return Success or SlvError::EepromError / SlvError::OutOfBounds.
   */
  [[nodiscard]] virtual Result<> read_bytes(size_t byte_offset,
                                            std::span<byte> buffer) = 0;

  /**
   * @brief Write SII bytes starting at byte offset.
   * @param byte_offset Byte offset inside EEPROM.
   * @param data Source bytes (`2` bytes typical for 1 SII word).
   * @return Success or SlvError::EepromError / SlvError::OutOfBounds.
   */
  [[nodiscard]] virtual Result<> write_bytes(size_t byte_offset,
                                             std::span<const byte> data) = 0;

  /**
   * @brief Apply EEPROM reload side-effects (configured alias, PDI ctrl, etc.).
   * @param reload_buffer Scratch output buffer receiving first reloaded bytes if applicable.
   * @return Success or SlvError::EepromError.
   */
  [[nodiscard]] virtual Result<> reload(std::span<byte> reload_buffer) = 0;
};

} // namespace resoes
