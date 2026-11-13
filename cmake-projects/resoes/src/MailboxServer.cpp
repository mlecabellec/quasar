/**
 * @file MailboxServer.cpp
 * @brief Pooled ETG.1000 Mailbox buffer state engine & SM0/SM1 protocol dispatcher.
 * @feature [TSK-20261113-001]
 */

#include "resoes/MailboxServer.hpp"
#include <array>
#include <chrono>

namespace resoes {

/// @brief Construct MailboxServer sizing slots from SlaveConfig.
MailboxServer::MailboxServer(IEscHardware &hardware, EscController &controller)
    : hardware_{hardware}, controller_{controller} {
  size_t slot_cnt = static_cast<size_t>(controller_.config().mailbox_slots);
  if (slot_cnt == 0U) {
    slot_cnt = constants::DEFAULT_MBX_BUFFERS;
  }
  slots_.resize(slot_cnt);
}

/// @brief Register an external mailbox protocol handler (e.g. CoE, FoE, EoE).
Result<> MailboxServer::register_handler(IMailboxProtocolHandler *handler) {
  if (handler == nullptr) {
    return std::unexpected(SlvError::InvalidParameter);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  handlers_.push_back(handler);
  return {};
}

/// @brief Reset all mailbox slots and registered protocol sessions cleanly.
void MailboxServer::reset() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return;
  }
  for (size_t idx = 0U; idx < slots_.size(); ++idx) {
    slots_[idx].state = MailboxSlotState::Idle;
    slots_[idx].length = 0U;
    slots_[idx].frame.clear();
  }
  last_rx_counter_ = 0U;
  next_tx_counter_ = 1U;
  for (size_t idx = 0U; idx < handlers_.size(); ++idx) {
    if (handlers_[idx] != nullptr) {
      handlers_[idx]->reset();
    }
  }
}

/// @brief Find index of an Idle mailbox slot (`SIZE_MAX` if none).
size_t MailboxServer::allocate_slot_locked() const noexcept {
  for (size_t idx = 0U; idx < slots_.size(); ++idx) {
    if (slots_[idx].state == MailboxSlotState::Idle) {
      return idx;
    }
  }
  return SIZE_MAX;
}

/// @brief Advance outgoing ETG.1000 mailbox counter (`1..7`).
uint8_t MailboxServer::consume_tx_counter_locked() noexcept {
  uint8_t current = next_tx_counter_;
  next_tx_counter_ += 1U;
  if (next_tx_counter_ > constants::MBX_MAX_COUNTER) {
    next_tx_counter_ = 1U;
  }
  return current;
}

/// @brief Build a 10-byte ETG.1000 Mailbox Error frame and stage it in a slot.
Result<> MailboxServer::queue_mbx_error_locked(uint16_t error_code) {
  size_t slot_idx = allocate_slot_locked();
  if (slot_idx == SIZE_MAX) {
    return std::unexpected(SlvError::MailboxFull);
  }
  std::vector<byte> frame{};
  frame.resize(constants::MBX_HEADER_SIZE + 4U, byte{0});
  MailboxHeader hdr{};
  hdr.length = 4U;
  hdr.address = 0U;
  hdr.channel = 0U;
  // Set error mailbox header fields cleanly.
  hdr.set_type_and_counter(constants::MBX_TYPE_ERROR,
                           consume_tx_counter_locked());
  (void)write_le16(frame, 0U, hdr.length);
  (void)write_le16(frame, 2U, hdr.address);
  frame[4U] = static_cast<byte>(hdr.channel);
  frame[5U] = static_cast<byte>(hdr.type_cnt);
  // Write MBX error service (0x0001) and error detail code.
  (void)write_le16(frame, 6U, 0x0001U);
  (void)write_le16(frame, 8U, error_code);
  slots_[slot_idx].state = MailboxSlotState::Again;
  slots_[slot_idx].mbx_type = constants::MBX_TYPE_ERROR;
  slots_[slot_idx].length = static_cast<uint16_t>(frame.size());
  slots_[slot_idx].frame = std::move(frame);
  return {};
}

/// @brief Write next queued `Again` slot into ESC SM1 buffer when SM1 is free.
Result<bool> MailboxServer::flush_tx_queue_locked() {
  Result<SyncManagerRegisters> sm1_res =
      controller_.sync_manager(constants::SM_INDEX_MBX_IN);
  if (sm1_res.has_value() == false) {
    return std::unexpected(sm1_res.error());
  }
  SyncManagerRegisters sm1 = sm1_res.value();
  // Read live SM1 status directly from hardware DPRAM register 0x080D.
  std::array<byte, 1U> st_raw{};
  Result<> st_res = hardware_.read(constants::ESCREG_SM1_STATUS, st_raw);
  if (st_res.has_value() == false) {
    return std::unexpected(st_res.error());
  }
  uint8_t sm1_status = std::to_integer<uint8_t>(st_raw[0U]);
  // Reclaim any prior OwnerEcat slot once Master has emptied SM1.
  if ((sm1_status & constants::SM_STATUS_MBX_FULL) == 0U) {
    for (size_t idx = 0U; idx < slots_.size(); ++idx) {
      if (slots_[idx].state == MailboxSlotState::OwnerEcat) {
        slots_[idx].state = MailboxSlotState::Idle;
        slots_[idx].frame.clear();
      }
    }
  } else {
    // Handle ETG.1000 SM1 repeat request when Master failed to receive response.
    if ((sm1.activate & constants::SM_ACTIVATE_REPEAT) != 0U) {
      for (size_t idx = 0U; idx < slots_.size(); ++idx) {
        if (slots_[idx].state == MailboxSlotState::OwnerEcat) {
          std::vector<byte> padded{};
          padded.resize(sm1.length, byte{0});
          size_t copy_n = std::min(slots_[idx].frame.size(), padded.size());
          for (size_t k = 0U; k < copy_n; ++k) {
            padded[k] = slots_[idx].frame[k];
          }
          Result<> wres = hardware_.write(sm1.physical_start, padded);
          if (wres.has_value() == false) {
            return std::unexpected(wres.error());
          }
          return true;
        }
      }
    }
    return false;
  }
  // Find first waiting `Again` slot and write padded frame into SM1.
  for (size_t idx = 0U; idx < slots_.size(); ++idx) {
    if (slots_[idx].state == MailboxSlotState::Again) {
      if (slots_[idx].frame.size() > sm1.length) {
        slots_[idx].state = MailboxSlotState::Idle;
        return std::unexpected(SlvError::BufferTooSmall);
      }
      std::vector<byte> padded{};
      padded.resize(sm1.length, byte{0});
      for (size_t k = 0U; k < slots_[idx].frame.size(); ++k) {
        padded[k] = slots_[idx].frame[k];
      }
      Result<> wres = hardware_.write(sm1.physical_start, padded);
      if (wres.has_value() == false) {
        return std::unexpected(wres.error());
      }
      slots_[idx].state = MailboxSlotState::OwnerEcat;
      return true;
    }
  }
  return false;
}

/// @brief Enqueue an unsolicited mailbox payload (e.g. CoE Emergency).
Result<> MailboxServer::enqueue_tx(uint8_t protocol_type,
                                   std::span<const byte> payload) {
  EscStateSnapshot snap = controller_.snapshot();
  if ((snap.active_mbx0 == false) || (snap.active_mbx1 == false)) {
    return std::unexpected(SlvError::InvalidState);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t slot_idx = allocate_slot_locked();
  if (slot_idx == SIZE_MAX) {
    return std::unexpected(SlvError::MailboxFull);
  }
  std::vector<byte> frame{};
  frame.resize(constants::MBX_HEADER_SIZE + payload.size(), byte{0});
  MailboxHeader hdr{};
  hdr.length = static_cast<uint16_t>(payload.size());
  hdr.address = snap.station_address;
  hdr.channel = 0U;
  hdr.set_type_and_counter(protocol_type, consume_tx_counter_locked());
  (void)write_le16(frame, 0U, hdr.length);
  (void)write_le16(frame, 2U, hdr.address);
  frame[4U] = static_cast<byte>(hdr.channel);
  frame[5U] = static_cast<byte>(hdr.type_cnt);
  for (size_t idx = 0U; idx < payload.size(); ++idx) {
    frame[constants::MBX_HEADER_SIZE + idx] = payload[idx];
  }
  slots_[slot_idx].state = MailboxSlotState::Again;
  slots_[slot_idx].mbx_type = protocol_type;
  slots_[slot_idx].length = static_cast<uint16_t>(frame.size());
  slots_[slot_idx].frame = std::move(frame);
  // Flush queued TX slot explicitly so std::expected works without C++23 monadic .transform (GCC 12).
  Result<bool> flushed = flush_tx_queue_locked();
  // Propagate flush failure before reporting mailbox enqueue success.
  if (flushed.has_value() == false) {
    return std::unexpected(flushed.error());
  }
  return {};
}

/// @brief Execute one mailbox service cycle: flush SM1 TX & read/dispatch SM0 RX.
Result<size_t> MailboxServer::process() {
  EscStateSnapshot snap = controller_.snapshot();
  if ((snap.active_mbx0 == false) || (snap.active_mbx1 == false)) {
    return 0U;
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  size_t serviced = 0U;
  Result<bool> flushed = flush_tx_queue_locked();
  if (flushed.has_value() == false) {
    return std::unexpected(flushed.error());
  }
  if (flushed.value() == true) {
    serviced += 1U;
  }
  // Inspect SM0 status to see if Master wrote a new mailbox request.
  std::array<byte, 1U> sm0_st_raw{};
  Result<> st_res = hardware_.read(constants::ESCREG_SM0_STATUS, sm0_st_raw);
  if (st_res.has_value() == false) {
    return std::unexpected(st_res.error());
  }
  uint8_t sm0_status = std::to_integer<uint8_t>(sm0_st_raw[0U]);
  if ((sm0_status & constants::SM_STATUS_MBX_FULL) == 0U) {
    return serviced;
  }
  Result<SyncManagerRegisters> sm0_res =
      controller_.sync_manager(constants::SM_INDEX_MBX_OUT);
  if (sm0_res.has_value() == false) {
    return std::unexpected(sm0_res.error());
  }
  SyncManagerRegisters sm0 = sm0_res.value();
  std::vector<byte> rx_buf{};
  rx_buf.resize(sm0.length, byte{0});
  Result<> rres = hardware_.read(sm0.physical_start, rx_buf);
  if (rres.has_value() == false) {
    return std::unexpected(rres.error());
  }
  serviced += 1U;
  // Parse 6-byte ETG.1000 Mailbox Header cleanly.
  if (rx_buf.size() < constants::MBX_HEADER_SIZE) {
    (void)queue_mbx_error_locked(constants::MBXERR_SIZE_TOO_SHORT);
    (void)flush_tx_queue_locked();
    return serviced;
  }
  MailboxHeader req_hdr{};
  req_hdr.length = read_le16(rx_buf, 0U).value_or(0U);
  req_hdr.address = read_le16(rx_buf, 2U).value_or(0U);
  req_hdr.channel = std::to_integer<uint8_t>(rx_buf[4U]);
  req_hdr.type_cnt = std::to_integer<uint8_t>(rx_buf[5U]);
  if ((req_hdr.length == 0U) ||
      ((constants::MBX_HEADER_SIZE + req_hdr.length) > rx_buf.size())) {
    (void)queue_mbx_error_locked(constants::MBXERR_INVALID_SIZE);
    (void)flush_tx_queue_locked();
    return serviced;
  }
  // Track SM0 mailbox counter cleanly.
  uint8_t rx_cnt = req_hdr.counter();
  if ((rx_cnt != 0U) && (rx_cnt == last_rx_counter_)) {
    // Duplicate counter can be dropped cleanly per ETG.1000.
  }
  last_rx_counter_ = rx_cnt;
  // Locate matching protocol handler for req_hdr.type().
  IMailboxProtocolHandler *target = nullptr;
  for (size_t idx = 0U; idx < handlers_.size(); ++idx) {
    if ((handlers_[idx] != nullptr) &&
        (handlers_[idx]->protocol_type() == req_hdr.type())) {
      target = handlers_[idx];
      break;
    }
  }
  if (target == nullptr) {
    (void)queue_mbx_error_locked(constants::MBXERR_UNSUPPORTED);
    (void)flush_tx_queue_locked();
    return serviced;
  }
  std::span<const byte> req_payload{
      rx_buf.data() + constants::MBX_HEADER_SIZE, req_hdr.length};
  std::vector<byte> tx_payload{};
  uint8_t tx_type = req_hdr.type();
  bool send_reply = false;
  uint16_t mbx_err = target->handle_request(req_hdr, req_payload, tx_payload,
                                            tx_type, send_reply);
  if (mbx_err != 0U) {
    (void)queue_mbx_error_locked(mbx_err);
  } else if (send_reply == true) {
    size_t slot_idx = allocate_slot_locked();
    if (slot_idx == SIZE_MAX) {
      return std::unexpected(SlvError::MailboxFull);
    }
    std::vector<byte> frame(constants::MBX_HEADER_SIZE + tx_payload.size(),
                            byte{0});
    MailboxHeader resp_hdr{};
    resp_hdr.length = static_cast<uint16_t>(tx_payload.size());
    resp_hdr.address = req_hdr.address;
    resp_hdr.channel = req_hdr.channel;
    resp_hdr.set_type_and_counter(tx_type, consume_tx_counter_locked());
    (void)write_le16(frame, 0U, resp_hdr.length);
    (void)write_le16(frame, 2U, resp_hdr.address);
    frame[4U] = static_cast<byte>(resp_hdr.channel);
    frame[5U] = static_cast<byte>(resp_hdr.type_cnt);
    for (size_t k = 0U; k < tx_payload.size(); ++k) {
      frame[constants::MBX_HEADER_SIZE + k] = tx_payload[k];
    }
    slots_[slot_idx].state = MailboxSlotState::Again;
    slots_[slot_idx].mbx_type = tx_type;
    slots_[slot_idx].length = static_cast<uint16_t>(frame.size());
    slots_[slot_idx].frame = std::move(frame);
  } else {
    // Request consumed cleanly without immediate TX reply.
  }
  Result<bool> post_flush = flush_tx_queue_locked();
  if (post_flush.has_value() == false) {
    return std::unexpected(post_flush.error());
  }
  return serviced;
}

/// @brief Query count of buffered/in-flight TX slots.
size_t MailboxServer::pending_tx_count() const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return 0U;
  }
  size_t count = 0U;
  for (size_t idx = 0U; idx < slots_.size(); ++idx) {
    if ((slots_[idx].state == MailboxSlotState::Again) ||
        (slots_[idx].state == MailboxSlotState::OwnerEcat)) {
      count += 1U;
    }
  }
  return count;
}

} // namespace resoes
