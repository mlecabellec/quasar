/**
 * @file SlaveApplication.hpp
 * @brief Top-level EtherCAT SubDevice runtime runner symmetrical to SOES ecat_slv.
 * @feature [TSK-20261113-001]
 */

#pragma once

#include "resoes/CoeServer.hpp"
#include "resoes/EepromEmulator.hpp"
#include "resoes/EscController.hpp"
#include "resoes/IEscHardware.hpp"
#include "resoes/MailboxServer.hpp"
#include "resoes/ObjectDictionary.hpp"
#include <cstdint>
#include <mutex>
#include <vector>

namespace resoes {

/**
 * @brief Flags controlling cyclic digital IO processing (`DIG_process` symmetry).
 * @feature [TSK-20261113-001]
 */
struct IoProcessFlags {
  bool process_inputs = true;   ///< Pack & write TxPDO (SM3)
  bool process_outputs = true;  ///< Read & unpack RxPDO (SM2)
  bool check_watchdog = true;   ///< Monitor SM2/PDI watchdog status
  bool invoke_app_hook = true;  ///< Run user cyclic application callback
};

/**
 * @brief Application-level cyclic PDO and safety callbacks.
 * @feature [TSK-20261113-001]
 */
class IApplicationCallbacks {
public:
  /// @brief Virtual destructor ensuring safe polymorphic cleanup [CS-0060.5].
  virtual ~IApplicationCallbacks() noexcept = default;

  /// @brief Called right after RxPDO (`SM2`) values have been unpacked into ObjectDictionary.
  virtual void on_rx_pdo_updated() = 0;

  /// @brief Called immediately before TxPDO (`SM3`) values are packed from ObjectDictionary.
  virtual void on_tx_pdo_prepare() = 0;

  /// @brief Called whenever SubDevice drops outputs (Watchdog / PREOP / INIT / Error).
  virtual void on_safe_outputs() = 0;

  /// @brief Called once per application cycle when `invoke_app_hook` is enabled.
  virtual void on_application_tick() = 0;
};

/**
 * @brief Coordinates ESC HAL, ESM, EEPROM, Mailbox, CoE, and cyclic PDO execution.
 * @feature [TSK-20261113-001] [CS-0010.46]
 */
class SlaveApplication final : public IEsmHooks {
public:
  /**
   * @brief Construct SubDevice stack components around provided ESC hardware & EEPROM.
   * @param hardware Non-null ESC hardware HAL.
   * @param eeprom Non-null SII EEPROM storage backend.
   * @param config Runtime SubDevice parameters.
   */
  SlaveApplication(IEscHardware &hardware, IEepromStorage &eeprom,
                   const SlaveConfig &config);

  /// @brief Default noexcept destructor [CS-0060.5].
  ~SlaveApplication() noexcept override = default;

  /// @brief Attach optional application PDO/lifecycle callback object.
  void set_callbacks(IApplicationCallbacks *callbacks);

  /// @brief Initialize hardware, controller, CoE server, and register mailbox handler.
  [[nodiscard]] Result<> init();

  /// @brief Execute one complete free-run polling cycle (`ecat_slv` symmetry).
  [[nodiscard]] Result<> step_freerun(IoProcessFlags flags = IoProcessFlags{});

  /// @brief Service non-cyclic AL/SM/EEPROM/Mailbox events (`ecat_slv_poll` symmetry).
  [[nodiscard]] Result<> step_poll();

  /// @brief Handle masked ESC AL Event bits (`ecat_slv_worker` symmetry).
  [[nodiscard]] Result<> handle_interrupt_events(uint16_t al_event_mask);

  /// @brief Execute cyclic SM2 RxPDO unpack, SM3 TxPDO pack, and Watchdog check.
  [[nodiscard]] Result<> process_digital_io(
      IoProcessFlags flags = IoProcessFlags{});

  /// @brief Access owned EscController.
  [[nodiscard]] EscController &esc() noexcept { return controller_; }
  /// @brief Access owned MailboxServer.
  [[nodiscard]] MailboxServer &mailbox() noexcept { return mailbox_; }
  /// @brief Access owned ObjectDictionary.
  [[nodiscard]] ObjectDictionary &od() noexcept { return od_; }
  /// @brief Access owned CoeServer.
  [[nodiscard]] CoeServer &coe() noexcept { return coe_; }
  /// @brief Access owned EepromEmulator.
  [[nodiscard]] EepromEmulator &eeprom() noexcept { return eeprom_; }

  /// @brief Compute dynamic RxPDO/TxPDO byte counts via CoeServer.
  [[nodiscard]] Result<> on_configure_pdo_sizes(
      uint16_t &rx_pdo_bytes, uint16_t &tx_pdo_bytes) override;
  /// @brief Notify application callbacks to enter safe outputs.
  void on_safe_output() override;

private:
  /// @brief ESC hardware interface.
  IEscHardware &hardware_;
  /// @brief Owned SII EEPROM emulator.
  EepromEmulator eeprom_;
  /// @brief Owned ESM/SyncManager/Watchdog controller.
  EscController controller_;
  /// @brief Owned Mailbox server engine.
  MailboxServer mailbox_;
  /// @brief Owned Object Dictionary.
  ObjectDictionary od_{};
  /// @brief Owned CoE SDO/PDO server.
  CoeServer coe_;
  /// @brief Optional application lifecycle callbacks.
  IApplicationCallbacks *callbacks_ = nullptr;
  /// @brief Reusable RxPDO scratch buffer (`SM2`).
  std::vector<byte> rx_pdo_buf_{};
  /// @brief Reusable TxPDO scratch buffer (`SM3`).
  std::vector<byte> tx_pdo_buf_{};
  /// @brief Timed mutex serializing cyclic runner execution [CS-0010.46].
  mutable std::timed_mutex mutex_{};
};

} // namespace resoes
