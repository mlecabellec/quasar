/**
 * @file EepromEmulator.hpp
 * @brief SII EEPROM 4-byte/8-byte read/write/reload emulation symmetrical to SOES esc_eep.
 * @feature [TSK-20261113-001]
 */

#pragma once

#include "resoes/IEepromStorage.hpp"
#include "resoes/IEscHardware.hpp"
#include "resoes/ResoesConstants.hpp"
#include <functional>
#include <mutex>
#include <vector>

namespace resoes {

/**
 * @brief In-memory EEPROM backing storage implementation suitable for virtual/RAM slaves.
 * @feature [TSK-20261113-001]
 */
class MemoryEepromStorage final : public IEepromStorage {
public:
  /// @brief Construct EEPROM image initialized with zeros to given byte capacity.
  explicit MemoryEepromStorage(
      size_t size_bytes = constants::EEP_DEFAULT_SIZE_BYTES);

  /// @brief Construct EEPROM image initialized from an existing binary SII image.
  explicit MemoryEepromStorage(std::vector<byte> initial_image);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~MemoryEepromStorage() noexcept override = default;

  /// @brief Return total EEPROM size in bytes.
  [[nodiscard]] size_t size_bytes() const noexcept override;

  /// @brief Read SII bytes from memory backing store.
  [[nodiscard]] Result<> read_bytes(size_t byte_offset,
                                    std::span<byte> buffer) override;

  /// @brief Write SII bytes into memory backing store.
  [[nodiscard]] Result<> write_bytes(size_t byte_offset,
                                     std::span<const byte> data) override;

  /// @brief Populate reload buffer with initial SII configuration bytes (`0x0000`+).
  [[nodiscard]] Result<> reload(std::span<byte> reload_buffer) override;

  /// @brief Read count of completed reload operations for diagnostics/tests.
  [[nodiscard]] uint32_t reload_count() const noexcept;

private:
  /// @brief Backing SII byte buffer.
  std::vector<byte> storage_{};
  /// @brief Counter tracking reload operations.
  uint32_t reload_count_ = 0U;
  /// @brief Timed mutex guarding EEPROM backing store [CS-0010.46].
  mutable std::timed_mutex mutex_{};
};

/**
 * @brief ESC SII EEPROM Emulation Controller servicing `0x0502`–`0x050F`.
 * @feature [TSK-20261113-001] Symmetrical to SOES EEP_process
 */
class EepromEmulator final {
public:
  /// @brief Alias for reload notification callback invoked after EEPROM reload.
  using ReloadCallback = std::move_only_function<Result<>(std::span<const byte>)>;

  /**
   * @brief Construct an EEPROM emulator bound to ESC hardware and EEPROM storage.
   * @param hardware Non-null ESC hardware reference.
   * @param storage Non-null SII storage backing reference.
   * @param read_size Granularity per read command (`4` or `8` bytes).
   */
  EepromEmulator(IEscHardware &hardware, IEepromStorage &storage,
                 uint8_t read_size = constants::EEP_READ_SIZE_8);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~EepromEmulator() noexcept = default;

  /**
   * @brief Configure read granularity (`4` or `8` bytes per READ cycle).
   * @param read_size Must be `4` or `8`.
   * @return Success or SlvError::InvalidParameter.
   */
  [[nodiscard]] Result<> set_read_size(uint8_t read_size);

  /// @brief Register an optional callback invoked upon `EEP_CMD_RELOAD`.
  void set_reload_callback(ReloadCallback callback);

  /**
   * @brief Execute one EEPROM emulation service cycle (`EEP_process` symmetry).
   * @return True if an EEPROM command was serviced, false if idle, or SlvError.
   */
  [[nodiscard]] Result<bool> step();

private:
  /// @brief Reference to ESC hardware peripheral.
  IEscHardware &hardware_;
  /// @brief Reference to backing SII storage.
  IEepromStorage &storage_;
  /// @brief Active read block size (`4` or `8`).
  uint8_t read_size_ = constants::EEP_READ_SIZE_8;
  /// @brief Optional reload hook.
  ReloadCallback reload_callback_{};
  /// @brief Timed mutex serializing emulator steps [CS-0010.46].
  mutable std::timed_mutex mutex_{};
};

} // namespace resoes
