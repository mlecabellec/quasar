/**
 * @file EscController.hpp
 * @brief Context-owned EtherCAT State Machine (ESM), SyncManager & Watchdog controller.
 * @feature [TSK-20261113-001] Symmetrical C++23 core to SOES esc.c (zero globals)
 */

#pragma once

#include "resoes/EscTypes.hpp"
#include "resoes/IEscHardware.hpp"
#include "resoes/ResoesConstants.hpp"
#include <array>
#include <functional>
#include <mutex>

namespace resoes {

/**
 * @brief Observer/policy hooks invoked by EscController on state/SM transitions.
 * @feature [TSK-20261113-001]
 */
class IEsmHooks {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~IEsmHooks() noexcept = default;

  /// @brief Compute expected RxPDO (`SM2`) and TxPDO (`SM3`) byte sizes before SAFEOP.
  [[nodiscard]] virtual Result<> on_configure_pdo_sizes(
      uint16_t &rx_pdo_bytes, uint16_t &tx_pdo_bytes) = 0;

  /// @brief Validate custom preconditions on `INIT->PREOP` transition.
  [[nodiscard]] virtual uint16_t on_preop_transition() { return constants::ALERR_NONE; }

  /// @brief Validate custom preconditions on `PREOP->SAFEOP` transition.
  [[nodiscard]] virtual uint16_t on_safeop_transition() { return constants::ALERR_NONE; }

  /// @brief Validate custom preconditions on `SAFEOP->OP` transition.
  [[nodiscard]] virtual uint16_t on_op_transition() { return constants::ALERR_NONE; }

  /// @brief Called when transitioning to `INIT` / `PREOP` / error to enter safe output state.
  virtual void on_safe_output() = 0;
};

/**
 * @brief Snapshot of active SubDevice state (symmetrical to `_ESCvar` without globals).
 * @feature [TSK-20261113-001]
 */
struct EscStateSnapshot {
  AlState al_state = AlState::Init;
  uint16_t al_error = constants::ALERR_NONE;
  bool al_error_flag = false;
  uint16_t station_address = 0U;
  uint16_t dl_status = 0U;
  uint16_t al_event = 0U;
  uint16_t rx_pdo_bytes = 0U;
  uint16_t tx_pdo_bytes = 0U;
  bool active_mbx0 = false;       ///< SM0 mailbox out active
  bool active_mbx1 = false;       ///< SM1 mailbox in active
  bool active_sm2 = false;        ///< SM2 RxPDO active
  bool active_sm3 = false;        ///< SM3 TxPDO active
  // Bootstrap and DC sync status
  bool boot_active = false;       ///< Bootstrap state active
  bool dc_sync_enabled = false;   ///< Distributed Clocks sync active
  uint8_t dc_sync_mode = 0U;      ///< Raw SYNC0/1 activation bits
  bool watchdog_expired = false;  ///< Process-data watchdog tripped
};

/**
 * @brief Manages SubDevice ESM transitions, SyncManagers SM0–3, Watchdog, and DC checks.
 * @feature [TSK-20261113-001] [CS-0010.46]
 */
class EscController final {
public:
  /**
   * @brief Construct EscController bound to ESC hardware and runtime SlaveConfig.
   * @param hardware Non-null ESC hardware HAL.
   * @param config Runtime SubDevice configuration.
   */
  EscController(IEscHardware &hardware, const SlaveConfig &config);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~EscController() noexcept = default;

  /// @brief Attach optional application hooks notified on transitions/safe outputs.
  void set_hooks(IEsmHooks *hooks);

  /// @brief Initialize ESC AL Status to `Init` and reset SyncManager state.
  [[nodiscard]] Result<> init();

  /// @brief Read `ALCONTROL` (`0x0120`) and process ETG.1000 ESM transition & Error ACK.
  [[nodiscard]] Result<> process_al_control();

  /// @brief Read `ALEVENT` (`0x0220`), service SM activations & SM2/SM3 validity.
  [[nodiscard]] Result<> process_sm_events();

  /// @brief Check ESC process-data watchdog (`0x0440`) and enforce safe state on expiry.
  [[nodiscard]] Result<bool> check_watchdog();

  /// @brief Validate Distributed Clocks `SYNC0`/`SYNC1` configuration registers.
  [[nodiscard]] Result<uint16_t> check_dc_sync();

  /// @brief Explicitly set AL Status Error register and update AL Status with error bit.
  [[nodiscard]] Result<> raise_al_error(AlState target_state, uint16_t error_code);

  /// @brief Manually force AL Status update inside ESC register `0x0130`.
  [[nodiscard]] Result<> write_al_status(AlState state, bool error_bit);

  /// @brief Return immutable thread-safe snapshot of current controller state.
  [[nodiscard]] EscStateSnapshot snapshot() const;

  /// @brief Return current SyncManager register snapshot (`index` `0`..`3`).
  [[nodiscard]] Result<SyncManagerRegisters> sync_manager(uint8_t index) const;

  /// @brief Return active SlaveConfig.
  [[nodiscard]] const SlaveConfig &config() const noexcept { return config_; }

private:
  /// @brief Hardware interface reference.
  IEscHardware &hardware_;
  /// @brief Runtime configuration.
  SlaveConfig config_{};
  /// @brief Optional application transition hooks.
  IEsmHooks *hooks_ = nullptr;
  /// @brief Cached SyncManager register descriptors (`SM0`..`SM3`).
  std::array<SyncManagerRegisters, constants::SM_COUNT> sms_{};
  /// @brief Current controller state snapshot.
  EscStateSnapshot state_{};
  /// @brief Timed mutex guarding all controller state mutations [CS-0010.46].
  mutable std::timed_mutex mutex_{};

  /// @brief Read all SM0..3 registers from ESC hardware (lock already held).
  [[nodiscard]] Result<> refresh_sync_managers_locked();

  /// @brief Validate & activate standard SM0/SM1 mailbox pair (`ESC_startmbx` symmetry).
  [[nodiscard]] uint16_t start_mailbox_locked(bool boot_mode);

  /// @brief Deactivate SM0/SM1 mailbox pair (`ESC_stopmbx` symmetry).
  void stop_mailbox_locked();

  /// @brief Validate & activate SM2 RxPDO outputs (`ESC_startoutput` symmetry).
  [[nodiscard]] uint16_t start_outputs_locked();

  /// @brief Deactivate SM2 RxPDO outputs (`ESC_stopoutput` symmetry).
  void stop_outputs_locked();

  /// @brief Validate & activate SM3 TxPDO inputs (`ESC_startinput` symmetry).
  [[nodiscard]] uint16_t start_inputs_locked();

  /// @brief Deactivate SM3 TxPDO inputs (`ESC_stopinput` symmetry).
  void stop_inputs_locked();

  /// @brief Execute transition dispatch from `INIT`, `PREOP`, `BOOT`, `SAFEOP`, or `OP`.
  [[nodiscard]] Result<> handle_transition_locked(uint16_t req_state, bool ack);

  /// @brief Update ESC `ALSTATUS` & `ALERROR` registers (lock already held).
  [[nodiscard]] Result<> commit_al_status_locked(AlState state, bool error_bit,
                                                 uint16_t error_code);

  /// @brief Check whether two address intervals overlap in ESC DPRAM.
  [[nodiscard]] static constexpr bool intervals_overlap(
      uint16_t start_a, uint16_t len_a, uint16_t start_b,
      uint16_t len_b) noexcept {
    if ((len_a == 0U) || (len_b == 0U)) {
      return false;
    }
    uint32_t end_a = static_cast<uint32_t>(start_a) + static_cast<uint32_t>(len_a);
    uint32_t end_b = static_cast<uint32_t>(start_b) + static_cast<uint32_t>(len_b);
    return (static_cast<uint32_t>(start_a) < end_b) &&
           (static_cast<uint32_t>(start_b) < end_a);
  }
};

} // namespace resoes
