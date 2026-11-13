/**
 * @file MemoryEscHardware.hpp
 * @brief Thread-safe 8 KiB in-memory ESC DPRAM simulator implementing IEscHardware.
 * @feature [TSK-20261113-001] Hardware-free SubDevice ESC simulation
 */

#pragma once

#include "resoes/IEscHardware.hpp"
#include "resoes/ResoesConstants.hpp"
#include <array>
#include <mutex>

namespace resoes {

/**
 * @brief In-memory dual-port RAM emulator for ESC registers, SyncManagers, and PDI events.
 * @feature [TSK-20261113-001]
 */
class MemoryEscHardware final : public IEscHardware {
public:
  /// @brief Construct zero-initialized simulated ESC DPRAM.
  MemoryEscHardware() noexcept;

  /// @brief Default noexcept destructor [CS-0060.5].
  ~MemoryEscHardware() noexcept override = default;

  /// @brief Initialize registers (DLStatus port0 link, EEPROM 8B cap).
  [[nodiscard]] Result<> init() override;

  /// @brief Reset all ESC registers and SM state to power-on defaults.
  [[nodiscard]] Result<> reset() override;

  /// @brief PDI-side read from ESC register/DPRAM space with SM handshake semantics.
  [[nodiscard]] Result<> read(uint16_t address, std::span<byte> buffer) override;

  /// @brief PDI-side write to ESC register/DPRAM space with SM handshake semantics.
  [[nodiscard]] Result<> write(uint16_t address,
                               std::span<const byte> data) override;

  /// @brief Convenience PDI read of little-endian uint16_t register.
  [[nodiscard]] Result<uint16_t> read_u16(uint16_t address) override;

  /// @brief Convenience PDI write of little-endian uint16_t register.
  [[nodiscard]] Result<> write_u16(uint16_t address, uint16_t value) override;

  /**
   * @brief Master/ECAT-side raw register read bypassing PDI-only side effects.
   * @param address ESC byte offset.
   * @param buffer Destination span.
   * @return Success or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<> ecat_read(uint16_t address, std::span<byte> buffer);

  /**
   * @brief Master/ECAT-side raw register write triggering appropriate AL Event flags.
   * @param address ESC byte offset.
   * @param data Source bytes.
   * @return Success or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<> ecat_write(uint16_t address,
                                    std::span<const byte> data);

  /**
   * @brief Master/ECAT-side helper to write 16-bit register and update AL Events.
   * @param address ESC register offset.
   * @param value Register value.
   * @return Success or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<> ecat_write_u16(uint16_t address, uint16_t value);

  /**
   * @brief Master/ECAT-side helper to read a 16-bit ESC register.
   * @param address ESC register offset.
   * @return 16-bit value or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<uint16_t> ecat_read_u16(uint16_t address);

  /**
   * @brief Master/ECAT-side write to a SyncManager DPRAM area (e.g. SM0/SM2).
   * @param sm_index SyncManager index (`0`..`3`).
   * @param payload Bytes written into the SM buffer.
   * @return Success or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<> ecat_write_sm(uint8_t sm_index,
                                       std::span<const byte> payload);

  /**
   * @brief Master/ECAT-side read from a SyncManager DPRAM area (e.g. SM1/SM3).
   * @param sm_index SyncManager index (`0`..`3`).
   * @param buffer Destination buffer matching expected transfer size.
   * @return Success or SlvError.
   * @exposed
   */
  [[nodiscard]] Result<> ecat_read_sm(uint8_t sm_index, std::span<byte> buffer);

private:
  /// @brief Shared DPRAM byte array (`ESC_MEMORY_SIZE_BYTES`).
  std::array<byte, constants::ESC_MEMORY_SIZE_BYTES> memory_{};
  /// @brief Timed mutex protecting all DPRAM and event mutations [CS-0010.46].
  mutable std::timed_mutex mutex_{};

  /// @brief Initialize default power-on ESC register values (lock already held).
  void reset_unlocked() noexcept;

  /// @brief Read 16-bit value directly from internal memory (lock already held).
  [[nodiscard]] uint16_t raw_read_u16(uint16_t address) const noexcept;

  /// @brief Write 16-bit value directly to internal memory (lock already held).
  void raw_write_u16(uint16_t address, uint16_t value) noexcept;

  /// @brief Set bits in AL Event register `0x0220` (lock already held).
  void set_al_event_bits(uint16_t bits) noexcept;

  /// @brief Clear bits in AL Event register `0x0220` (lock already held).
  void clear_al_event_bits(uint16_t bits) noexcept;
};

} // namespace resoes
