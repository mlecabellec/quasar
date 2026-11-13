/**
 * @file EepromEmulator.cpp
 * @brief SII EEPROM 4B/8B emulation implementation symmetrical to SOES esc_eep.c.
 * @feature [TSK-20261113-001]
 */

#include "resoes/EepromEmulator.hpp"
#include <array>
#include <chrono>

namespace resoes {

/// @brief Construct blank in-memory EEPROM buffer of specified byte size.
MemoryEepromStorage::MemoryEepromStorage(size_t size_bytes)
    : storage_(size_bytes, byte{0}) {}

/// @brief Construct in-memory EEPROM buffer from initial binary image.
MemoryEepromStorage::MemoryEepromStorage(std::vector<byte> initial_image)
    : storage_{std::move(initial_image)} {}

/// @brief Return total backing size in bytes.
size_t MemoryEepromStorage::size_bytes() const noexcept {
  return storage_.size();
}

/// @brief Thread-safe SII byte read.
Result<> MemoryEepromStorage::read_bytes(size_t byte_offset,
                                         std::span<byte> buffer) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  if ((byte_offset > storage_.size()) ||
      ((storage_.size() - byte_offset) < buffer.size())) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  // Copy requested EEPROM slice into caller span.
  for (size_t idx = 0U; idx < buffer.size(); ++idx) {
    buffer[idx] = storage_[byte_offset + idx];
  }
  return {};
}

/// @brief Thread-safe SII byte write.
Result<> MemoryEepromStorage::write_bytes(size_t byte_offset,
                                          std::span<const byte> data) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  if ((byte_offset > storage_.size()) ||
      ((storage_.size() - byte_offset) < data.size())) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  for (size_t idx = 0U; idx < data.size(); ++idx) {
    storage_[byte_offset + idx] = data[idx];
  }
  return {};
}

/// @brief Reload initial EEPROM header words into buffer.
Result<> MemoryEepromStorage::reload(std::span<byte> reload_buffer) {
  Result<> res = read_bytes(0U, reload_buffer);
  if (res.has_value() == false) {
    return res;
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  reload_count_ += 1U;
  return {};
}

/// @brief Return number of completed reload operations.
uint32_t MemoryEepromStorage::reload_count() const noexcept {
  return reload_count_;
}

/// @brief Construct EepromEmulator bound to ESC hardware and storage backend.
EepromEmulator::EepromEmulator(IEscHardware &hardware, IEepromStorage &storage,
                               uint8_t read_size)
    : hardware_{hardware}, storage_{storage} {
  if ((read_size == constants::EEP_READ_SIZE_4) ||
      (read_size == constants::EEP_READ_SIZE_8)) {
    read_size_ = read_size;
  }
}

/// @brief Update active SII read granularity (`4` or `8` bytes).
Result<> EepromEmulator::set_read_size(uint8_t read_size) {
  if ((read_size != constants::EEP_READ_SIZE_4) &&
      (read_size != constants::EEP_READ_SIZE_8)) {
    return std::unexpected(SlvError::InvalidParameter);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  read_size_ = read_size;
  return {};
}

/// @brief Register move-only callback triggered after reload.
void EepromEmulator::set_reload_callback(ReloadCallback callback) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == true) {
    reload_callback_ = std::move(callback);
  }
}

/// @brief Execute one SII EEPROM emulation service pass (`EEP_process`).
Result<bool> EepromEmulator::step() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  Result<uint16_t> stat_res = hardware_.read_u16(constants::ESCREG_EECONTSTAT);
  if (stat_res.has_value() == false) {
    return std::unexpected(stat_res.error());
  }
  uint16_t status_reg = stat_res.value();
  uint16_t cmd = static_cast<uint16_t>(status_reg & constants::EEP_CMD_MASK);
  if (cmd == constants::EEP_CMD_IDLE) {
    return false;
  }
  // Read target SII word address (byte offset = word_addr * 2).
  Result<uint16_t> addr_res = hardware_.read_u16(constants::ESCREG_EEADDR);
  if (addr_res.has_value() == false) {
    return std::unexpected(addr_res.error());
  }
  size_t byte_offset =
      static_cast<size_t>(addr_res.value()) * constants::EEP_WORD_SIZE_BYTES;
  uint16_t next_status =
      static_cast<uint16_t>(status_reg &
                            static_cast<uint16_t>(~(constants::EEP_CMD_MASK |
                                                    constants::EEP_STAT_BUSY |
                                                    constants::EEP_STAT_ERROR_MASK)));
  // Dispatch EEPROM command cleanly with unconditional else fallback.
  if (cmd == constants::EEP_CMD_READ) {
    std::array<byte, constants::EEP_READ_SIZE_8> buf{};
    std::span<byte> slice{buf.data(), static_cast<size_t>(read_size_)};
    Result<> rres = storage_.read_bytes(byte_offset, slice);
    if (rres.has_value() == false) {
      next_status = static_cast<uint16_t>(next_status | constants::EEP_STAT_ACK_ERR);
    } else {
      Result<> wres = hardware_.write(constants::ESCREG_EEDATA, slice);
      if (wres.has_value() == false) {
        return std::unexpected(wres.error());
      }
    }
  } else if (cmd == constants::EEP_CMD_WRITE) {
    std::array<byte, constants::EEP_WORD_SIZE_BYTES> word_buf{};
    Result<> rres = hardware_.read(constants::ESCREG_EEDATA, word_buf);
    if (rres.has_value() == false) {
      return std::unexpected(rres.error());
    }
    Result<> wres = storage_.write_bytes(byte_offset, word_buf);
    if (wres.has_value() == false) {
      next_status = static_cast<uint16_t>(next_status | constants::EEP_STAT_WRITE_ERR);
    }
  } else if (cmd == constants::EEP_CMD_RELOAD) {
    std::array<byte, constants::EEP_READ_SIZE_8> reload_buf{};
    std::span<byte> slice{reload_buf.data(), static_cast<size_t>(read_size_)};
    Result<> rres = storage_.reload(slice);
    if (rres.has_value() == false) {
      next_status = static_cast<uint16_t>(next_status | constants::EEP_STAT_ACK_ERR);
    } else {
      Result<> wres = hardware_.write(constants::ESCREG_EEDATA, slice);
      if (wres.has_value() == false) {
        return std::unexpected(wres.error());
      }
      if (reload_callback_ != nullptr) {
        Result<> cb_res = reload_callback_(slice);
        if (cb_res.has_value() == false) {
          next_status =
              static_cast<uint16_t>(next_status | constants::EEP_STAT_WRITE_ERR);
        }
      }
    }
  } else {
    next_status = static_cast<uint16_t>(next_status | constants::EEP_STAT_ACK_ERR);
  }
  if (read_size_ == constants::EEP_READ_SIZE_8) {
    next_status = static_cast<uint16_t>(next_status | constants::EEP_STAT_READBYTES_8);
  }
  Result<> commit_res =
      hardware_.write_u16(constants::ESCREG_EECONTSTAT, next_status);
  if (commit_res.has_value() == false) {
    return std::unexpected(commit_res.error());
  }
  return true;
}

} // namespace resoes
