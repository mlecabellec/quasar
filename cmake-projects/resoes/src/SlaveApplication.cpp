/**
 * @file SlaveApplication.cpp
 * @brief Top-level EtherCAT SubDevice runner symmetrical to SOES ecat_slv.c.
 * @feature [TSK-20261113-001]
 */

#include "resoes/SlaveApplication.hpp"
#include <chrono>

namespace resoes {

/// @brief Construct SlaveApplication wiring hardware, EEPROM, ESM, Mailbox & CoE.
SlaveApplication::SlaveApplication(IEscHardware &hardware, IEepromStorage &eeprom,
                                   const SlaveConfig &config)
    : hardware_{hardware},
      eeprom_{hardware, eeprom, config.eeprom_read_size},
      controller_{hardware, config},
      mailbox_{hardware, controller_},
      coe_{od_, controller_} {
  rx_pdo_buf_.resize(config.max_rx_pdo_bytes, byte{0});
  tx_pdo_buf_.resize(config.max_tx_pdo_bytes, byte{0});
}

/// @brief Attach optional application lifecycle/PDO callbacks.
void SlaveApplication::set_callbacks(IApplicationCallbacks *callbacks) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == true) {
    callbacks_ = callbacks;
  }
}

/// @brief Initialize hardware, controller hooks, and CoE mailbox handler.
Result<> SlaveApplication::init() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  Result<> hw_res = hardware_.init();
  if (hw_res.has_value() == false) {
    return hw_res;
  }
  controller_.set_hooks(this);
  Result<> esc_res = controller_.init();
  if (esc_res.has_value() == false) {
    return esc_res;
  }
  mailbox_.reset();
  return mailbox_.register_handler(&coe_);
}

/// @brief Query dynamic PDO byte sizes from CoE Object Dictionary (`0x1C12`/`0x1C13`).
Result<> SlaveApplication::on_configure_pdo_sizes(uint16_t &rx_pdo_bytes,
                                                  uint16_t &tx_pdo_bytes) {
  return coe_.compute_pdo_sizes_bytes(rx_pdo_bytes, tx_pdo_bytes);
}

/// @brief Forward safe-output notification cleanly to application callbacks.
void SlaveApplication::on_safe_output() {
  if (callbacks_ != nullptr) {
    callbacks_->on_safe_outputs();
  }
}

/// @brief Handle specific masked AL Event flags (`ecat_slv_worker` symmetry).
Result<> SlaveApplication::handle_interrupt_events(uint16_t al_event_mask) {
  if ((al_event_mask & constants::ESC_AL_EVENT_SM_CHANGE) != 0U) {
    Result<> sm_res = controller_.process_sm_events();
    if (sm_res.has_value() == false) {
      return sm_res;
    }
  }
  if ((al_event_mask & constants::ESC_AL_EVENT_AL_CONTROL) != 0U) {
    Result<> al_res = controller_.process_al_control();
    if (al_res.has_value() == false) {
      return al_res;
    }
  }
  if ((al_event_mask & constants::ESC_AL_EVENT_EEP_CMD) != 0U) {
    Result<bool> eep_res = eeprom_.step();
    if (eep_res.has_value() == false) {
      return std::unexpected(eep_res.error());
    }
  }
  Result<size_t> mbx_res = mailbox_.process();
  if (mbx_res.has_value() == false) {
    return std::unexpected(mbx_res.error());
  }
  return {};
}

/// @brief Execute non-cyclic event polling cycle (`ecat_slv_poll` symmetry).
Result<> SlaveApplication::step_poll() {
  Result<uint16_t> ev_res = hardware_.read_u16(constants::ESCREG_ALEVENT);
  if (ev_res.has_value() == false) {
    return std::unexpected(ev_res.error());
  }
  return handle_interrupt_events(ev_res.value());
}

/// @brief Service cyclic SM2 outputs, SM3 inputs, and process-data watchdog (`DIG_process`).
Result<> SlaveApplication::process_digital_io(IoProcessFlags flags) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  EscStateSnapshot snap = controller_.snapshot();
  if (flags.check_watchdog == true) {
    Result<bool> wd_res = controller_.check_watchdog();
    if (wd_res.has_value() == false) {
      return std::unexpected(wd_res.error());
    }
  }
  // Unpack incoming SM2 RxPDO only when SubDevice is Operational with SM2 active.
  if ((flags.process_outputs == true) && (snap.al_state == AlState::Op) &&
      (snap.active_sm2 == true) && (snap.rx_pdo_bytes > 0U)) {
    if (rx_pdo_buf_.size() < snap.rx_pdo_bytes) {
      rx_pdo_buf_.resize(snap.rx_pdo_bytes, byte{0});
    }
    std::span<byte> rx_slice{rx_pdo_buf_.data(), snap.rx_pdo_bytes};
    Result<> rres = hardware_.read(controller_.config().sm2_address, rx_slice);
    if (rres.has_value() == false) {
      return rres;
    }
    Result<uint32_t> ures = coe_.unpack_rx_pdo(rx_slice);
    if (ures.has_value() == false) {
      return std::unexpected(ures.error());
    }
    if (callbacks_ != nullptr) {
      callbacks_->on_rx_pdo_updated();
    }
  }
  // Invoke application hook when enabled.
  if ((flags.invoke_app_hook == true) && (callbacks_ != nullptr)) {
    callbacks_->on_application_tick();
  }
  // Pack outgoing SM3 TxPDO when SubDevice is SafeOp or Op with SM3 active.
  bool tx_allowed = (snap.al_state == AlState::SafeOp) || (snap.al_state == AlState::Op);
  if ((flags.process_inputs == true) && (tx_allowed == true) &&
      (snap.active_sm3 == true) && (snap.tx_pdo_bytes > 0U)) {
    if (callbacks_ != nullptr) {
      callbacks_->on_tx_pdo_prepare();
    }
    if (tx_pdo_buf_.size() < snap.tx_pdo_bytes) {
      tx_pdo_buf_.resize(snap.tx_pdo_bytes, byte{0});
    }
    std::fill(tx_pdo_buf_.begin(), tx_pdo_buf_.end(), byte{0});
    std::span<byte> tx_slice{tx_pdo_buf_.data(), snap.tx_pdo_bytes};
    Result<uint32_t> pres = coe_.pack_tx_pdo(tx_slice);
    if (pres.has_value() == false) {
      return std::unexpected(pres.error());
    }
    Result<> wres = hardware_.write(controller_.config().sm3_address, tx_slice);
    if (wres.has_value() == false) {
      return wres;
    }
  }
  return {};
}

/// @brief Execute one full free-run SubDevice loop tick (`ecat_slv` symmetry).
Result<> SlaveApplication::step_freerun(IoProcessFlags flags) {
  Result<> poll_res = step_poll();
  if (poll_res.has_value() == false) {
    return poll_res;
  }
  return process_digital_io(flags);
}

} // namespace resoes
