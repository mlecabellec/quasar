/**
 * @file MemoryEscHardware.cpp
 * @brief Simulated ESC dual-port RAM with SyncManager & AL Event side effects.
 * @feature [TSK-20261113-001]
 */

#include "resoes/hal/MemoryEscHardware.hpp"
#include <algorithm>
#include <chrono>

namespace resoes {

namespace {
constexpr uint16_t DEFAULT_DL_STATUS = 0x0051U; // Port0 link up + PDI ok
constexpr uint16_t DEFAULT_WD_STATUS = 0x0001U; // Watchdog active/valid initially
} // namespace

/// @brief Construct MemoryEscHardware and reset DPRAM defaults.
MemoryEscHardware::MemoryEscHardware() noexcept { reset_unlocked(); }

/// @brief Initialize simulated ESC hardware.
Result<> MemoryEscHardware::init() { return reset(); }

/// @brief Reset all simulated ESC registers cleanly under timed lock.
Result<> MemoryEscHardware::reset() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  reset_unlocked();
  return {};
}

/// @brief Initialize power-on ESC register values.
void MemoryEscHardware::reset_unlocked() noexcept {
  memory_.fill(byte{0});
  raw_write_u16(constants::ESCREG_DLSTATUS, DEFAULT_DL_STATUS);
  raw_write_u16(constants::ESCREG_WDSTATUS, DEFAULT_WD_STATUS);
  raw_write_u16(constants::ESCREG_ALSTATUS, constants::AL_STATE_INIT);
  // Advertise 8-byte SII EEPROM read support at 0x0502.
  raw_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_STAT_READBYTES_8);
}

/// @brief Read raw 16-bit LE word from DPRAM array (caller holds mutex).
uint16_t MemoryEscHardware::raw_read_u16(uint16_t address) const noexcept {
  size_t off = static_cast<size_t>(address);
  if ((off + 1U) >= memory_.size()) {
    return 0U;
  }
  uint16_t lo = std::to_integer<uint16_t>(memory_[off]);
  uint16_t hi = std::to_integer<uint16_t>(memory_[off + 1U]);
  return static_cast<uint16_t>(lo | static_cast<uint16_t>(hi << 8U));
}

/// @brief Write raw 16-bit LE word into DPRAM array (caller holds mutex).
void MemoryEscHardware::raw_write_u16(uint16_t address,
                                      uint16_t value) noexcept {
  size_t off = static_cast<size_t>(address);
  if ((off + 1U) >= memory_.size()) {
    return;
  }
  memory_[off] = static_cast<byte>(value & 0xFFU);
  memory_[off + 1U] = static_cast<byte>((value >> 8U) & 0xFFU);
}

/// @brief Set AL Event bits inside register `0x0220`.
void MemoryEscHardware::set_al_event_bits(uint16_t bits) noexcept {
  uint16_t cur = raw_read_u16(constants::ESCREG_ALEVENT);
  raw_write_u16(constants::ESCREG_ALEVENT, static_cast<uint16_t>(cur | bits));
}

/// @brief Clear AL Event bits inside register `0x0220`.
void MemoryEscHardware::clear_al_event_bits(uint16_t bits) noexcept {
  uint16_t cur = raw_read_u16(constants::ESCREG_ALEVENT);
  uint16_t mask = static_cast<uint16_t>(~bits);
  raw_write_u16(constants::ESCREG_ALEVENT, static_cast<uint16_t>(cur & mask));
}

/// @brief PDI-side read from ESC register/DPRAM with SM0 full clear side effect.
Result<> MemoryEscHardware::read(uint16_t address, std::span<byte> buffer) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t end_off = static_cast<size_t>(address) + buffer.size();
  if (end_off > memory_.size()) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  // Copy requested DPRAM window cleanly into output span.
  for (size_t idx = 0U; idx < buffer.size(); ++idx) {
    buffer[idx] = memory_[static_cast<size_t>(address) + idx];
  }
  // Reading ALCONTROL acknowledges AL Control change event bit in ESC hardware.
  if (address == constants::ESCREG_ALCONTROL) {
    clear_al_event_bits(constants::ESC_AL_EVENT_AL_CONTROL);
  }
  // Reading the last byte of SM0 clears Mailbox Full bit & AL Event SM0 bit.
  uint16_t sm0_start = raw_read_u16(constants::ESCREG_SM0);
  uint16_t sm0_len = raw_read_u16(constants::ESCREG_SM0 + 2U);
  if ((sm0_len > 0U) && (address <= sm0_start) &&
      (end_off >= (static_cast<size_t>(sm0_start) + sm0_len))) {
    uint8_t st = std::to_integer<uint8_t>(memory_[constants::ESCREG_SM0_STATUS]);
    memory_[constants::ESCREG_SM0_STATUS] = static_cast<byte>(
        st & static_cast<uint8_t>(~constants::SM_STATUS_MBX_FULL));
    clear_al_event_bits(constants::ESC_DL_EVENT_SM0_FULL);
  }
  return {};
}

/// @brief PDI-side write to ESC register/DPRAM with SM1 mailbox-full side effect.
Result<> MemoryEscHardware::write(uint16_t address,
                                  std::span<const byte> data) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t end_off = static_cast<size_t>(address) + data.size();
  if (end_off > memory_.size()) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  // Copy payload bytes into simulated ESC memory.
  for (size_t idx = 0U; idx < data.size(); ++idx) {
    memory_[static_cast<size_t>(address) + idx] = data[idx];
  }
  // Acknowledge EEPROM command when PDI updates EECONTSTAT.
  if (address == constants::ESCREG_EECONTSTAT) {
    clear_al_event_bits(constants::ESC_AL_EVENT_EEP_CMD);
  }
  // Writing SM1 buffer through its final byte marks SM1 full for ECAT read.
  uint16_t sm1_start = raw_read_u16(constants::ESCREG_SM0 + 8U);
  uint16_t sm1_len = raw_read_u16(constants::ESCREG_SM0 + 10U);
  if ((sm1_len > 0U) && (address >= sm1_start) &&
      (end_off >= (static_cast<size_t>(sm1_start) + sm1_len))) {
    uint8_t st = std::to_integer<uint8_t>(memory_[constants::ESCREG_SM1_STATUS]);
    memory_[constants::ESCREG_SM1_STATUS] =
        static_cast<byte>(st | constants::SM_STATUS_MBX_FULL);
    clear_al_event_bits(constants::ESC_DL_EVENT_SM1_READ);
  }
  return {};
}

/// @brief Convenience 16-bit PDI read.
Result<uint16_t> MemoryEscHardware::read_u16(uint16_t address) {
  std::array<byte, 2U> tmp{};
  Result<> res = read(address, tmp);
  if (res.has_value() == false) {
    return std::unexpected(res.error());
  }
  return read_le16(tmp, 0U);
}

/// @brief Convenience 16-bit PDI write.
Result<> MemoryEscHardware::write_u16(uint16_t address, uint16_t value) {
  std::array<byte, 2U> tmp{};
  Result<> enc = write_le16(tmp, 0U, value);
  if (enc.has_value() == false) {
    return enc;
  }
  return write(address, tmp);
}

/// @brief Master/ECAT-side register read.
Result<> MemoryEscHardware::ecat_read(uint16_t address, std::span<byte> buffer) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t end_off = static_cast<size_t>(address) + buffer.size();
  if (end_off > memory_.size()) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  for (size_t idx = 0U; idx < buffer.size(); ++idx) {
    buffer[idx] = memory_[static_cast<size_t>(address) + idx];
  }
  return {};
}

/// @brief Master/ECAT-side register write raising ESC AL Events cleanly.
Result<> MemoryEscHardware::ecat_write(uint16_t address,
                                       std::span<const byte> data) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t end_off = static_cast<size_t>(address) + data.size();
  if (end_off > memory_.size()) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  for (size_t idx = 0U; idx < data.size(); ++idx) {
    memory_[static_cast<size_t>(address) + idx] = data[idx];
  }
  // Raise corresponding ESC AL Event bits depending on targeted register.
  if (address == constants::ESCREG_ALCONTROL) {
    set_al_event_bits(constants::ESC_AL_EVENT_AL_CONTROL);
  } else if ((address >= constants::ESCREG_SM0) &&
             (address < (constants::ESCREG_SM0 + 32U))) {
    set_al_event_bits(constants::ESC_AL_EVENT_SM_CHANGE);
  } else if (address == constants::ESCREG_EECONTSTAT) {
    uint16_t cmd_reg = raw_read_u16(constants::ESCREG_EECONTSTAT);
    if ((cmd_reg & constants::EEP_CMD_MASK) != 0U) {
      raw_write_u16(constants::ESCREG_EECONTSTAT,
                    static_cast<uint16_t>(cmd_reg | constants::EEP_STAT_BUSY));
      set_al_event_bits(constants::ESC_AL_EVENT_EEP_CMD);
    }
  } else {
    // Other register writes require no extra AL Event assertion.
  }
  return {};
}

/// @brief Master/ECAT-side 16-bit write wrapper.
Result<> MemoryEscHardware::ecat_write_u16(uint16_t address, uint16_t value) {
  std::array<byte, 2U> tmp{};
  Result<> enc = write_le16(tmp, 0U, value);
  if (enc.has_value() == false) {
    return enc;
  }
  return ecat_write(address, tmp);
}

/// @brief Master/ECAT-side 16-bit read wrapper.
Result<uint16_t> MemoryEscHardware::ecat_read_u16(uint16_t address) {
  std::array<byte, 2U> tmp{};
  Result<> res = ecat_read(address, tmp);
  if (res.has_value() == false) {
    return std::unexpected(res.error());
  }
  return read_le16(tmp, 0U);
}

/// @brief Master/ECAT write into SyncManager buffer (SM0 mailbox or SM2 RxPDO).
Result<> MemoryEscHardware::ecat_write_sm(uint8_t sm_index,
                                          std::span<const byte> payload) {
  if (sm_index >= constants::SM_COUNT) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  uint16_t sm_base =
      static_cast<uint16_t>(constants::ESCREG_SM0 +
                            (static_cast<uint16_t>(sm_index) * 8U));
  uint16_t start = raw_read_u16(sm_base);
  uint16_t len = raw_read_u16(static_cast<uint16_t>(sm_base + 2U));
  if ((payload.size() > len) ||
      ((static_cast<size_t>(start) + len) > memory_.size())) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  // Write incoming payload into SyncManager DPRAM area.
  for (size_t idx = 0U; idx < payload.size(); ++idx) {
    memory_[static_cast<size_t>(start) + idx] = payload[idx];
  }
  // Zero-pad any unused trailing bytes up to configured SM length.
  for (size_t idx = payload.size(); idx < len; ++idx) {
    memory_[static_cast<size_t>(start) + idx] = byte{0};
  }
  if (sm_index == constants::SM_INDEX_MBX_OUT) {
    uint8_t st = std::to_integer<uint8_t>(memory_[constants::ESCREG_SM0_STATUS]);
    memory_[constants::ESCREG_SM0_STATUS] =
        static_cast<byte>(st | constants::SM_STATUS_MBX_FULL);
    set_al_event_bits(constants::ESC_DL_EVENT_SM0_FULL);
  } else if (sm_index == constants::SM_INDEX_PDO_OUT) {
    set_al_event_bits(constants::ESC_DL_EVENT_SM2_EVENT);
  } else {
    // Other SM write events not asserted by SM write helper.
  }
  return {};
}

/// @brief Master/ECAT read from SyncManager buffer (SM1 mailbox or SM3 TxPDO).
Result<> MemoryEscHardware::ecat_read_sm(uint8_t sm_index,
                                         std::span<byte> buffer) {
  if (sm_index >= constants::SM_COUNT) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  uint16_t sm_base =
      static_cast<uint16_t>(constants::ESCREG_SM0 +
                            (static_cast<uint16_t>(sm_index) * 8U));
  uint16_t start = raw_read_u16(sm_base);
  uint16_t len = raw_read_u16(static_cast<uint16_t>(sm_base + 2U));
  if ((buffer.size() > len) ||
      ((static_cast<size_t>(start) + buffer.size()) > memory_.size())) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  for (size_t idx = 0U; idx < buffer.size(); ++idx) {
    buffer[idx] = memory_[static_cast<size_t>(start) + idx];
  }
  if (sm_index == constants::SM_INDEX_MBX_IN) {
    uint8_t st = std::to_integer<uint8_t>(memory_[constants::ESCREG_SM1_STATUS]);
    memory_[constants::ESCREG_SM1_STATUS] = static_cast<byte>(
        st & static_cast<uint8_t>(~constants::SM_STATUS_MBX_FULL));
    set_al_event_bits(constants::ESC_DL_EVENT_SM1_READ);
  } else {
    // Reading SM3 does not alter SM1 status bits.
  }
  return {};
}

} // namespace resoes
