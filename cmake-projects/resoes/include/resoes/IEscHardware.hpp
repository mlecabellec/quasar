/**
 * @file IEscHardware.hpp
 * @brief Abstract PDI HAL for EtherCAT Slave Controller (ESC) registers and DPRAM.
 * @feature [TSK-20261113-001] Symmetrical C++23 HAL to SOES esc_hw
 */

#pragma once

#include "resoes/common.hpp"
#include <cstdint>
#include <span>

namespace resoes {

/**
 * @brief Pure virtual RAII interface for PDI-side ESC register and dual-port RAM access.
 * @feature [TSK-20261113-001]
 */
class IEscHardware {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~IEscHardware() noexcept = default;

  /**
   * @brief Initialize the ESC hardware interface.
   * @return Success or hardware error.
   */
  [[nodiscard]] virtual Result<> init() = 0;

  /**
   * @brief Reset the ESC hardware peripheral.
   * @return Success or hardware error.
   */
  [[nodiscard]] virtual Result<> reset() = 0;

  /**
   * @brief Read bytes from ESC register or DPRAM space starting at offset.
   * @param address ESC byte offset (`0x0000`–`0x0FFF`+).
   * @param buffer Destination span receiving the read bytes.
   * @return Success or hardware/bounds error.
   */
  [[nodiscard]] virtual Result<> read(uint16_t address,
                                      std::span<byte> buffer) = 0;

  /**
   * @brief Write bytes into ESC register or DPRAM space starting at offset.
   * @param address ESC byte offset (`0x0000`–`0x0FFF`+).
   * @param data Source span containing bytes to write.
   * @return Success or hardware/bounds error.
   */
  [[nodiscard]] virtual Result<> write(uint16_t address,
                                       std::span<const byte> data) = 0;

  /**
   * @brief Convenience helper to read a little-endian 16-bit ESC register.
   * @param address ESC register offset.
   * @return 16-bit value or SlvError.
   */
  [[nodiscard]] virtual Result<uint16_t> read_u16(uint16_t address) = 0;

  /**
   * @brief Convenience helper to write a little-endian 16-bit ESC register.
   * @param address ESC register offset.
   * @param value Value to write.
   * @return Success or SlvError.
   */
  [[nodiscard]] virtual Result<> write_u16(uint16_t address,
                                           uint16_t value) = 0;
};

} // namespace resoes
