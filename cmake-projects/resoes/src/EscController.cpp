/**
 * @file EscController.cpp
 * @brief Context-owned EtherCAT State Machine (ESM), SM0–3 & Watchdog implementation.
 * @feature [TSK-20261113-001]
 */

#include "resoes/EscController.hpp"
#include <array>
#include <chrono>

namespace resoes {

namespace {
constexpr uint16_t SM_BLOCK_BYTES = constants::SM_COUNT * constants::ESCREG_SM_STRIDE;
} // namespace

/// @brief Construct EscController around ESC HAL and runtime configuration.
EscController::EscController(IEscHardware &hardware, const SlaveConfig &config)
    : hardware_{hardware}, config_{config} {}

/// @brief Set optional state/transition hooks.
void EscController::set_hooks(IEsmHooks *hooks) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == true) {
    hooks_ = hooks;
  }
}

/// @brief Initialize controller state and reset AL Status to Init.
Result<> EscController::init() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  state_ = EscStateSnapshot{};
  state_.al_state = AlState::Init;
  Result<> st_res =
      commit_al_status_locked(AlState::Init, false, constants::ALERR_NONE);
  if (st_res.has_value() == false) {
    return st_res;
  }
  return refresh_sync_managers_locked();
}

/// @brief Read all SyncManager registers (`0x0800..0x081F`) into cache.
Result<> EscController::refresh_sync_managers_locked() {
  std::array<byte, SM_BLOCK_BYTES> raw{};
  Result<> rres = hardware_.read(constants::ESCREG_SM0, raw);
  if (rres.has_value() == false) {
    return rres;
  }
  // Decode 4 x 8-byte SyncManager register blocks cleanly without reinterpret_cast.
  for (size_t idx = 0U; idx < constants::SM_COUNT; ++idx) {
    size_t base = idx * constants::ESCREG_SM_STRIDE;
    Result<uint16_t> start_r = read_le16(raw, base);
    Result<uint16_t> len_r = read_le16(raw, base + 2U);
    if ((start_r.has_value() == false) || (len_r.has_value() == false)) {
      return std::unexpected(SlvError::BufferTooSmall);
    }
    sms_[idx].physical_start = start_r.value();
    sms_[idx].length = len_r.value();
    sms_[idx].control = std::to_integer<uint8_t>(raw[base + 4U]);
    sms_[idx].status = std::to_integer<uint8_t>(raw[base + 5U]);
    sms_[idx].activate = std::to_integer<uint8_t>(raw[base + 6U]);
    sms_[idx].pdi_control = std::to_integer<uint8_t>(raw[base + 7U]);
  }
  return {};
}

/// @brief Validate and activate SM0 + SM1 mailbox pair (`ESC_startmbx` symmetry).
uint16_t EscController::start_mailbox_locked(bool boot_mode) {
  const SyncManagerRegisters &SM0 = sms_[constants::SM_INDEX_MBX_OUT];
  const SyncManagerRegisters &SM1 = sms_[constants::SM_INDEX_MBX_IN];
  uint16_t exp_addr0 =
      (boot_mode == true) ? config_.mbx0_boot_address : config_.mbx0_address;
  uint16_t exp_addr1 =
      (boot_mode == true) ? config_.mbx1_boot_address : config_.mbx1_address;
  uint16_t max_len =
      (boot_mode == true) ? config_.mbx_boot_size : config_.mbx_size;
  uint16_t err_code = (boot_mode == true) ? constants::ALERR_INVALID_MAILBOX_CFG_BOOT
                                          : constants::ALERR_INVALID_MAILBOX_CFG_PREOP;
  // Validate SM enable flags and lengths.
  if (((SM0.activate & constants::SM_ACTIVATE_ENABLE) == 0U) ||
      ((SM1.activate & constants::SM_ACTIVATE_ENABLE) == 0U)) {
    return err_code;
  }
  if ((SM0.length < constants::MBX_HEADER_SIZE) ||
      (SM1.length < constants::MBX_HEADER_SIZE) ||
      (SM0.length > max_len) || (SM1.length > max_len)) {
    return err_code;
  }
  // Check start addresses, mailbox mode, direction, and mutual non-overlap.
  bool dir0_ok = (SM0.control & constants::SM_CTRL_DIR_MASK) == constants::SM_CTRL_DIR_WRITE;
  bool dir1_ok = (SM1.control & constants::SM_CTRL_DIR_MASK) == constants::SM_CTRL_DIR_READ;
  bool mode0_ok = (SM0.control & constants::SM_CTRL_MODE_MASK) == constants::SM_CTRL_MODE_MAILBOX;
  bool mode1_ok = (SM1.control & constants::SM_CTRL_MODE_MASK) == constants::SM_CTRL_MODE_MAILBOX;
  if ((dir0_ok == false) || (dir1_ok == false) || (mode0_ok == false) ||
      (mode1_ok == false) || (SM0.physical_start != exp_addr0) ||
      (SM1.physical_start != exp_addr1) ||
      (intervals_overlap(SM0.physical_start, SM0.length, SM1.physical_start,
                         SM1.length) == true)) {
    return err_code;
  }
  // Clear PDI deactivation bits cleanly inside ESC hardware.
  std::array<byte, 1U> clear_pdi{byte{0}};
  (void)hardware_.write(constants::ESCREG_SM0_PDI_CTRL, clear_pdi);
  (void)hardware_.write(constants::ESCREG_SM1_PDI_CTRL, clear_pdi);
  state_.active_mbx0 = true;
  state_.active_mbx1 = true;
  state_.boot_active = boot_mode;
  return constants::ALERR_NONE;
}

/// @brief Deactivate SM0 + SM1 mailbox channels cleanly.
void EscController::stop_mailbox_locked() {
  std::array<byte, 1U> disable_pdi{byte{constants::SM_PDI_CTRL_DEACTIVATE}};
  (void)hardware_.write(constants::ESCREG_SM0_PDI_CTRL, disable_pdi);
  (void)hardware_.write(constants::ESCREG_SM1_PDI_CTRL, disable_pdi);
  state_.active_mbx0 = false;
  state_.active_mbx1 = false;
  state_.boot_active = false;
}

/// @brief Validate and activate SM2 RxPDO outputs (`ESC_startoutput` symmetry).
uint16_t EscController::start_outputs_locked() {
  const SyncManagerRegisters &SM2 = sms_[constants::SM_INDEX_PDO_OUT];
  if (state_.rx_pdo_bytes == 0U) {
    state_.active_sm2 = ((SM2.activate & constants::SM_ACTIVATE_ENABLE) != 0U);
    return constants::ALERR_NONE;
  }
  // Require active SM2 with matching physical start, exact RxPDO size, and write dir.
  bool enabled = (SM2.activate & constants::SM_ACTIVATE_ENABLE) != 0U;
  bool dir_ok =
      (SM2.control & constants::SM_CTRL_DIR_MASK) == constants::SM_CTRL_DIR_WRITE;
  if ((enabled == false) || (dir_ok == false) ||
      (SM2.physical_start != config_.sm2_address) ||
      (SM2.length != state_.rx_pdo_bytes)) {
    return constants::ALERR_INVALID_OUTPUT_CFG;
  }
  std::array<byte, 1U> clear_pdi{byte{0}};
  (void)hardware_.write(constants::ESCREG_SM2_PDI_CTRL, clear_pdi);
  state_.active_sm2 = true;
  state_.watchdog_expired = false;
  return constants::ALERR_NONE;
}

/// @brief Deactivate SM2 outputs and signal user safe-output hook.
void EscController::stop_outputs_locked() {
  std::array<byte, 1U> disable_pdi{byte{constants::SM_PDI_CTRL_DEACTIVATE}};
  (void)hardware_.write(constants::ESCREG_SM2_PDI_CTRL, disable_pdi);
  state_.active_sm2 = false;
  if (hooks_ != nullptr) {
    hooks_->on_safe_output();
  }
}

/// @brief Validate and activate SM3 TxPDO inputs (`ESC_startinput` symmetry).
uint16_t EscController::start_inputs_locked() {
  const SyncManagerRegisters &SM3 = sms_[constants::SM_INDEX_PDO_IN];
  if (state_.tx_pdo_bytes == 0U) {
    state_.active_sm3 = ((SM3.activate & constants::SM_ACTIVATE_ENABLE) != 0U);
    return constants::ALERR_NONE;
  }
  bool enabled = (SM3.activate & constants::SM_ACTIVATE_ENABLE) != 0U;
  bool dir_ok =
      (SM3.control & constants::SM_CTRL_DIR_MASK) == constants::SM_CTRL_DIR_READ;
  if ((enabled == false) || (dir_ok == false) ||
      (SM3.physical_start != config_.sm3_address) ||
      (SM3.length != state_.tx_pdo_bytes)) {
    return constants::ALERR_INVALID_INPUT_CFG;
  }
  std::array<byte, 1U> clear_pdi{byte{0}};
  (void)hardware_.write(constants::ESCREG_SM3_PDI_CTRL, clear_pdi);
  state_.active_sm3 = true;
  return constants::ALERR_NONE;
}

/// @brief Deactivate SM3 inputs cleanly.
void EscController::stop_inputs_locked() {
  std::array<byte, 1U> disable_pdi{byte{constants::SM_PDI_CTRL_DEACTIVATE}};
  (void)hardware_.write(constants::ESCREG_SM3_PDI_CTRL, disable_pdi);
  state_.active_sm3 = false;
}

/// @brief Commit AL Status & AL Error Code registers inside ESC hardware.
Result<> EscController::commit_al_status_locked(AlState state, bool error_bit,
                                                uint16_t error_code) {
  state_.al_state = state;
  state_.al_error = error_code;
  state_.al_error_flag = error_bit;
  uint16_t raw_status = static_cast<uint16_t>(state);
  if (error_bit == true) {
    raw_status = static_cast<uint16_t>(raw_status | constants::AL_STATE_ERROR_BIT);
  }
  Result<> err_res = hardware_.write_u16(constants::ESCREG_ALERROR, error_code);
  if (err_res.has_value() == false) {
    return err_res;
  }
  return hardware_.write_u16(constants::ESCREG_ALSTATUS, raw_status);
}

/// @brief Process ETG.1000 ESM state transition & Error ACK (`ESC_state`).
Result<> EscController::process_al_control() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  Result<uint16_t> ctrl_res = hardware_.read_u16(constants::ESCREG_ALCONTROL);
  if (ctrl_res.has_value() == false) {
    return std::unexpected(ctrl_res.error());
  }
  uint16_t raw_ctrl = ctrl_res.value();
  uint16_t req_state = static_cast<uint16_t>(raw_ctrl & constants::AL_STATE_MASK);
  bool ack = (raw_ctrl & constants::AL_STATE_ACK_BIT) != 0U;
  Result<> sm_res = refresh_sync_managers_locked();
  if (sm_res.has_value() == false) {
    return sm_res;
  }
  // Ignore redundant requests while Error bit is latched and ACK is absent.
  if ((state_.al_error_flag == true) && (ack == false)) {
    return {};
  }
  return handle_transition_locked(req_state, ack);
}

/// @brief Execute state machine transition matrix matching ETG.1000 Figure 26.
Result<> EscController::handle_transition_locked(uint16_t req_state, bool ack) {
  AlState cur = state_.al_state;
  // Clear latched error indicator on explicit master Error ACK or Init request.
  if ((ack == true) && (req_state <= static_cast<uint16_t>(cur))) {
    state_.al_error_flag = false;
    state_.al_error = constants::ALERR_NONE;
  }
  // Always allow transitions back to INIT cleanly.
  if (req_state == constants::AL_STATE_INIT) {
    stop_outputs_locked();
    stop_inputs_locked();
    stop_mailbox_locked();
    return commit_al_status_locked(AlState::Init, false, constants::ALERR_NONE);
  }
  // Dispatch by current state with full branch coverage [CS-0060.3].
  if (cur == AlState::Init) {
    if (req_state == constants::AL_STATE_PREOP) {
      uint16_t mbx_err = start_mailbox_locked(false);
      uint16_t hook_err = (hooks_ != nullptr) ? hooks_->on_preop_transition()
                                              : constants::ALERR_NONE;
      uint16_t final_err = (mbx_err != constants::ALERR_NONE) ? mbx_err : hook_err;
      if (final_err != constants::ALERR_NONE) {
        stop_mailbox_locked();
        return commit_al_status_locked(AlState::Init, true, final_err);
      }
      return commit_al_status_locked(AlState::PreOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_BOOT) {
      if (config_.boot_supported == false) {
        return commit_al_status_locked(AlState::Init, true,
                                       constants::ALERR_BOOT_NOT_SUPPORTED);
      }
      uint16_t mbx_err = start_mailbox_locked(true);
      if (mbx_err != constants::ALERR_NONE) {
        stop_mailbox_locked();
        return commit_al_status_locked(AlState::Init, true, mbx_err);
      }
      return commit_al_status_locked(AlState::Boot, false, constants::ALERR_NONE);
    } else if ((req_state == constants::AL_STATE_SAFEOP) ||
               (req_state == constants::AL_STATE_OP)) {
      return commit_al_status_locked(AlState::Init, true,
                                     constants::ALERR_INVALID_STATE_CHANGE);
    } else {
      return commit_al_status_locked(AlState::Init, true,
                                     constants::ALERR_UNKNOWN_STATE);
    }
  } else if (cur == AlState::Boot) {
    return commit_al_status_locked(AlState::Boot, true,
                                   constants::ALERR_INVALID_STATE_CHANGE);
  } else if (cur == AlState::PreOp) {
    if (req_state == constants::AL_STATE_PREOP) {
      return commit_al_status_locked(AlState::PreOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_SAFEOP) {
      if (hooks_ != nullptr) {
        Result<> sz_res = hooks_->on_configure_pdo_sizes(state_.rx_pdo_bytes,
                                                         state_.tx_pdo_bytes);
        if (sz_res.has_value() == false) {
          return commit_al_status_locked(AlState::PreOp, true,
                                         constants::ALERR_INVALID_SM_CFG);
        }
      }
      uint16_t in_err = start_inputs_locked();
      if (in_err != constants::ALERR_NONE) {
        stop_inputs_locked();
        return commit_al_status_locked(AlState::PreOp, true, in_err);
      }
      uint16_t out_err = start_outputs_locked();
      if (out_err != constants::ALERR_NONE) {
        stop_outputs_locked();
        stop_inputs_locked();
        return commit_al_status_locked(AlState::PreOp, true, out_err);
      }
      uint16_t dc_err = check_dc_sync().value_or(constants::ALERR_DC_INVALID_SYNC_CFG);
      uint16_t hook_err = (hooks_ != nullptr) ? hooks_->on_safeop_transition()
                                              : constants::ALERR_NONE;
      uint16_t final_err = (dc_err != constants::ALERR_NONE) ? dc_err : hook_err;
      if (final_err != constants::ALERR_NONE) {
        stop_outputs_locked();
        stop_inputs_locked();
        return commit_al_status_locked(AlState::PreOp, true, final_err);
      }
      return commit_al_status_locked(AlState::SafeOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_OP) {
      return commit_al_status_locked(AlState::PreOp, true,
                                     constants::ALERR_INVALID_STATE_CHANGE);
    } else {
      return commit_al_status_locked(AlState::PreOp, true,
                                     constants::ALERR_UNKNOWN_STATE);
    }
  } else if (cur == AlState::SafeOp) {
    if (req_state == constants::AL_STATE_PREOP) {
      stop_outputs_locked();
      stop_inputs_locked();
      return commit_al_status_locked(AlState::PreOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_SAFEOP) {
      return commit_al_status_locked(AlState::SafeOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_OP) {
      uint16_t hook_err = (hooks_ != nullptr) ? hooks_->on_op_transition()
                                              : constants::ALERR_NONE;
      if (hook_err != constants::ALERR_NONE) {
        return commit_al_status_locked(AlState::SafeOp, true, hook_err);
      }
      return commit_al_status_locked(AlState::Op, false, constants::ALERR_NONE);
    } else {
      return commit_al_status_locked(AlState::SafeOp, true,
                                     constants::ALERR_UNKNOWN_STATE);
    }
  } else if (cur == AlState::Op) {
    if (req_state == constants::AL_STATE_PREOP) {
      stop_outputs_locked();
      stop_inputs_locked();
      return commit_al_status_locked(AlState::PreOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_SAFEOP) {
      if (hooks_ != nullptr) {
        hooks_->on_safe_output();
      }
      return commit_al_status_locked(AlState::SafeOp, false, constants::ALERR_NONE);
    } else if (req_state == constants::AL_STATE_OP) {
      return commit_al_status_locked(AlState::Op, false, constants::ALERR_NONE);
    } else {
      return commit_al_status_locked(AlState::Op, true,
                                     constants::ALERR_UNKNOWN_STATE);
    }
  } else {
    return commit_al_status_locked(AlState::Init, true, constants::ALERR_UNKNOWN_STATE);
  }
}

/// @brief Check SM change AL Events and detect mid-run SM invalidation (`ESC_sm_act_event`).
Result<> EscController::process_sm_events() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  Result<> sm_res = refresh_sync_managers_locked();
  if (sm_res.has_value() == false) {
    return sm_res;
  }
  // If SM0/1 changed out from under PreOp+, revert cleanly to Init with error.
  if ((state_.al_state != AlState::Init) &&
      (((sms_[constants::SM_INDEX_MBX_OUT].activate & constants::SM_ACTIVATE_ENABLE) == 0U) ||
       ((sms_[constants::SM_INDEX_MBX_IN].activate & constants::SM_ACTIVATE_ENABLE) == 0U))) {
    stop_outputs_locked();
    stop_inputs_locked();
    stop_mailbox_locked();
    return commit_al_status_locked(AlState::Init, true,
                                   constants::ALERR_INVALID_MAILBOX_CFG_PREOP);
  }
  return {};
}

/// @brief Check process-data watchdog (`0x0440`) and drop OP->SafeOp on expiry.
Result<bool> EscController::check_watchdog() {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  Result<uint16_t> wd_res = hardware_.read_u16(constants::ESCREG_WDSTATUS);
  if (wd_res.has_value() == false) {
    return std::unexpected(wd_res.error());
  }
  // Bit 0 == 0 indicates process data watchdog elapsed while outputs active.
  bool expired = (wd_res.value() & 0x0001U) == 0U;
  if ((expired == true) && (state_.al_state == AlState::Op)) {
    state_.watchdog_expired = true;
    if (hooks_ != nullptr) {
      hooks_->on_safe_output();
    }
    Result<> st_res = commit_al_status_locked(AlState::SafeOp, true,
                                              constants::ALERR_WATCHDOG);
    if (st_res.has_value() == false) {
      return std::unexpected(st_res.error());
    }
    return true;
  }
  state_.watchdog_expired = expired;
  return expired;
}

/// @brief Validate Distributed Clocks `0x0981` sync configuration.
Result<uint16_t> EscController::check_dc_sync() {
  Result<uint16_t> sync_res = hardware_.read_u16(constants::ESCREG_CYCLIC_UNIT_CTRL);
  if (sync_res.has_value() == false) {
    return std::unexpected(sync_res.error());
  }
  uint8_t sync_act = static_cast<uint8_t>((sync_res.value() >> 8U) & 0xFFU);
  // Check SYNC0/SYNC1 enable bit (bit 0) and SYNC0 cycle > 0 if enabled.
  if ((sync_act & 0x01U) != 0U) {
    std::array<byte, 4U> cyc0{};
    Result<> rres = hardware_.read(constants::ESCREG_SYNC0_CYCLE, cyc0);
    if (rres.has_value() == false) {
      return std::unexpected(rres.error());
    }
    uint32_t cycle_ns = read_le32(cyc0, 0U).value_or(0U);
    if (cycle_ns == 0U) {
      return constants::ALERR_DC_INVALID_SYNC_CFG;
    }
    state_.dc_sync_enabled = true;
    state_.dc_sync_mode = sync_act;
  } else {
    state_.dc_sync_enabled = false;
    state_.dc_sync_mode = 0U;
  }
  return constants::ALERR_NONE;
}

/// @brief Public wrapper to latch an AL Status Error cleanly.
Result<> EscController::raise_al_error(AlState target_state, uint16_t error_code) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  return commit_al_status_locked(target_state, true, error_code);
}

/// @brief Public wrapper to write AL Status register cleanly.
Result<> EscController::write_al_status(AlState state, bool error_bit) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  return commit_al_status_locked(state, error_bit, constants::ALERR_NONE);
}

/// @brief Thread-safe immutable copy of controller state.
EscStateSnapshot EscController::snapshot() const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return EscStateSnapshot{};
  }
  return state_;
}

/// @brief Thread-safe SyncManager register snapshot (`index` `0`..`3`).
Result<SyncManagerRegisters> EscController::sync_manager(uint8_t index) const {
  if (index >= constants::SM_COUNT) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  return sms_[index];
}

} // namespace resoes
