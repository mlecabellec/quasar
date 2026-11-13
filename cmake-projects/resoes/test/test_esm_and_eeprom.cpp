/**
 * @file test_esm_and_eeprom.cpp
 * @brief Hardware-free unit tests for resoes ESM transitions, Watchdog & SII EEPROM emulation.
 * @feature [TSK-20261113-001]
 */

#include "resoes/EepromEmulator.hpp"
#include "resoes/EscController.hpp"
#include "resoes/SlaveApplication.hpp"
#include "resoes/hal/MemoryEscHardware.hpp"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <iostream>

using namespace resoes;

namespace {

/// @brief Always-active runtime assertion helper safe under Release (-DNDEBUG) builds.
void check_true(bool cond, const char *label) {
  if (cond == false) {
    std::cerr << "CHECK FAILED: " << label << std::endl;
    std::abort();
  }
}

/// @brief Minimal ESM/App test observer counting safe-output triggers.
class TestAppHooks final : public IApplicationCallbacks {
public:
  uint32_t safe_output_calls = 0U; ///< Counter for safe-output events
  uint32_t rx_updates = 0U;        ///< Counter for SM2 RxPDO unpacks
  uint32_t tx_prepares = 0U;       ///< Counter for SM3 TxPDO packs
  uint32_t ticks = 0U;             ///< Counter for app ticks

  /// @brief Track RxPDO unpack notification.
  void on_rx_pdo_updated() override { rx_updates += 1U; }
  /// @brief Track TxPDO pack notification.
  void on_tx_pdo_prepare() override { tx_prepares += 1U; }
  /// @brief Track safe-output assertion count.
  void on_safe_outputs() override { safe_output_calls += 1U; }
  /// @brief Track application tick count.
  void on_application_tick() override { ticks += 1U; }
};

/// @brief Helper writing 8-byte SyncManager block cleanly from Master side.
void write_ecat_sm(MemoryEscHardware &hw, uint8_t sm_idx, uint16_t start,
                   uint16_t length, uint8_t ctrl, uint8_t activate) {
  std::array<byte, 8U> raw{};
  (void)write_le16(raw, 0U, start);
  (void)write_le16(raw, 2U, length);
  raw[4U] = static_cast<byte>(ctrl);
  raw[5U] = byte{0};
  raw[6U] = static_cast<byte>(activate);
  raw[7U] = byte{0};
  uint16_t addr =
      static_cast<uint16_t>(constants::ESCREG_SM0 +
                            (static_cast<uint16_t>(sm_idx) * 8U));
  check_true(hw.ecat_write(addr, raw).has_value() == true, "ecat_write SM");
}

/// @brief Verify 4-byte and 8-byte SII EEPROM READ, WRITE, RELOAD & out-of-bounds ACK error.
void test_eeprom_emulation() {
  MemoryEscHardware hw{};
  check_true(hw.init().has_value() == true, "hw init");
  std::vector<byte> img{};
  img.resize(256U, byte{0});
  (void)write_le32(img, 0x10U, 0x12345678U);
  (void)write_le32(img, 0x14U, 0xCAFEBABEU);
  MemoryEepromStorage storage{img};
  EepromEmulator emu{hw, storage, constants::EEP_READ_SIZE_8};

  bool reloaded = false;
  emu.set_reload_callback([&reloaded](std::span<const byte> hdr) -> Result<> {
    reloaded = (hdr.size() == constants::EEP_READ_SIZE_8);
    return {};
  });

  // Trigger 8-byte READ at word address 0x0008 (byte offset 0x0010).
  check_true(hw.ecat_write_u16(constants::ESCREG_EEADDR, 0x0008U).has_value(), "eeaddr");
  check_true(hw.ecat_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_CMD_READ)
                 .has_value(), "ee read cmd");
  Result<bool> step_res = emu.step();
  check_true((step_res.has_value() == true) && (step_res.value() == true), "eep step 1");

  std::array<byte, 8U> read_back{};
  check_true(hw.ecat_read(constants::ESCREG_EEDATA, read_back).has_value(), "eedata read");
  check_true(read_le32(read_back, 0U).value_or(0U) == 0x12345678U, "sii word 0");
  check_true(read_le32(read_back, 4U).value_or(0U) == 0xCAFEBABEU, "sii word 1");

  // Write 2-byte word 0xBEEF to word address 0x0020 (byte offset 0x0040).
  std::array<byte, 2U> wdata{};
  (void)write_le16(wdata, 0U, 0xBEEFU);
  check_true(hw.ecat_write(constants::ESCREG_EEDATA, wdata).has_value(), "eedata wr");
  check_true(hw.ecat_write_u16(constants::ESCREG_EEADDR, 0x0020U).has_value(), "eeaddr wr");
  check_true(hw.ecat_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_CMD_WRITE)
                 .has_value(), "ee write cmd");
  check_true(emu.step().value_or(false) == true, "eep write step");

  // Read back written word in 4-byte mode.
  check_true(emu.set_read_size(constants::EEP_READ_SIZE_4).has_value(), "set 4B mode");
  check_true(hw.ecat_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_CMD_READ)
                 .has_value(), "ee read 4B cmd");
  check_true(emu.step().value_or(false) == true, "eep read 4B step");
  std::array<byte, 4U> small_back{};
  check_true(hw.ecat_read(constants::ESCREG_EEDATA, small_back).has_value(), "read 4B");
  check_true(read_le16(small_back, 0U).value_or(0U) == 0xBEEFU, "verify 0xBEEF");

  // Trigger RELOAD and confirm callback invocation & count.
  check_true(emu.set_read_size(constants::EEP_READ_SIZE_8).has_value(), "set 8B mode");
  check_true(hw.ecat_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_CMD_RELOAD)
                 .has_value(), "reload cmd");
  check_true(emu.step().value_or(false) == true, "reload step");
  check_true(reloaded == true, "reload cb invoked");
  check_true(storage.reload_count() == 1U, "reload count == 1");

  // Verify out-of-bounds READ latches EEP_STAT_ACK_ERR without crashing.
  check_true(hw.ecat_write_u16(constants::ESCREG_EEADDR, 0x0FFFU).has_value(), "oob addr");
  check_true(hw.ecat_write_u16(constants::ESCREG_EECONTSTAT, constants::EEP_CMD_READ)
                 .has_value(), "oob read");
  check_true(emu.step().value_or(false) == true, "oob step");
  uint16_t err_stat = hw.ecat_read_u16(constants::ESCREG_EECONTSTAT).value_or(0U);
  check_true((err_stat & constants::EEP_STAT_ACK_ERR) != 0U, "oob ACK err latched");
}

/// @brief Verify ESM state transitions, invalid jump error ACK, SM checks & watchdog expiry.
void test_esm_and_watchdog() {
  MemoryEscHardware hw{};
  MemoryEepromStorage eep{};
  SlaveConfig cfg{};
  cfg.boot_supported = true;
  SlaveApplication app{hw, eep, cfg};
  TestAppHooks hooks{};
  app.set_callbacks(&hooks);
  check_true(app.init().has_value(), "app init");

  // Define 1 byte RxPDO output (0x7000) and 1 byte TxPDO input (0x6000) via 0x1C12/0x1C13.
  std::array<byte, 1U> zero1{byte{0}};
  std::array<byte, 1U> cnt1{byte{1}};
  std::array<byte, 2U> map1600{};
  (void)write_le16(map1600, 0U, 0x1600U);
  std::array<byte, 2U> map1a00{};
  (void)write_le16(map1a00, 0U, 0x1A00U);
  std::array<byte, 4U> rx_entry{};
  (void)write_le32(rx_entry, 0U, 0x70000008U);
  std::array<byte, 4U> tx_entry{};
  (void)write_le32(tx_entry, 0U, 0x60000008U);

  check_true(app.od().add_variable(0x7000U, OdDataType::Unsigned8, 8U,
                                   od_access::READ_WRITE_ALL | od_access::RX_PDO_MAP,
                                   "Out", zero1).has_value(), "add 0x7000");
  check_true(app.od().add_variable(0x6000U, OdDataType::Unsigned8, 8U,
                                   od_access::READ_ALL | od_access::TX_PDO_MAP,
                                   "In", zero1).has_value(), "add 0x6000");
  // Populate RxPDO mapping 0x1600.
  OdObjectEntry obj1600{};
  obj1600.index = 0x1600U;
  obj1600.max_subindex = 1U;
  obj1600.subindices.push_back(
      OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "N",
                      std::vector<byte>{cnt1.begin(), cnt1.end()}});
  obj1600.subindices.push_back(
      OdSubindexEntry{1U, OdDataType::Unsigned32, 32U, od_access::READ_ALL, "E1",
                      std::vector<byte>{rx_entry.begin(), rx_entry.end()}});
  check_true(app.od().register_object(std::move(obj1600)).has_value(), "reg 0x1600");

  // Populate TxPDO mapping 0x1A00.
  OdObjectEntry obj1a00{};
  obj1a00.index = 0x1A00U;
  obj1a00.max_subindex = 1U;
  obj1a00.subindices.push_back(
      OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "N",
                      std::vector<byte>{cnt1.begin(), cnt1.end()}});
  obj1a00.subindices.push_back(
      OdSubindexEntry{1U, OdDataType::Unsigned32, 32U, od_access::READ_ALL, "E1",
                      std::vector<byte>{tx_entry.begin(), tx_entry.end()}});
  check_true(app.od().register_object(std::move(obj1a00)).has_value(), "reg 0x1A00");

  // Register SM2 & SM3 assignment objects 0x1C12 and 0x1C13.
  OdObjectEntry obj1c12{};
  obj1c12.index = constants::OD_IDX_SM2_ASSIGN;
  obj1c12.max_subindex = 1U;
  obj1c12.subindices.push_back(
      OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "N",
                      std::vector<byte>{cnt1.begin(), cnt1.end()}});
  obj1c12.subindices.push_back(
      OdSubindexEntry{1U, OdDataType::Unsigned16, 16U, od_access::READ_ALL, "P0",
                      std::vector<byte>{map1600.begin(), map1600.end()}});
  check_true(app.od().register_object(std::move(obj1c12)).has_value(), "reg 0x1C12");

  OdObjectEntry obj1c13{};
  obj1c13.index = constants::OD_IDX_SM3_ASSIGN;
  obj1c13.max_subindex = 1U;
  obj1c13.subindices.push_back(
      OdSubindexEntry{0U, OdDataType::Unsigned8, 8U, od_access::READ_ALL, "N",
                      std::vector<byte>{cnt1.begin(), cnt1.end()}});
  obj1c13.subindices.push_back(
      OdSubindexEntry{1U, OdDataType::Unsigned16, 16U, od_access::READ_ALL, "P0",
                      std::vector<byte>{map1a00.begin(), map1a00.end()}});
  check_true(app.od().register_object(std::move(obj1c13)).has_value(), "reg 0x1C13");

  // Invalid direct Init -> Op request must reject with InvalidStateChange.
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_OP)
                 .has_value(), "req invalid op");
  check_true(app.step_freerun().has_value(), "step invalid op");
  uint16_t st = hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U);
  uint16_t err = hw.ecat_read_u16(constants::ESCREG_ALERROR).value_or(0U);
  check_true(st == (constants::AL_STATE_INIT | constants::AL_STATE_ERROR_BIT),
             "init+err latched");
  check_true(err == constants::ALERR_INVALID_STATE_CHANGE, "err == 0x0011");

  // Acknowledge error back to Init.
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL,
                               constants::AL_STATE_INIT | constants::AL_STATE_ACK_BIT)
                 .has_value(), "ack init");
  check_true(app.step_freerun().has_value(), "step ack");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U) ==
                 constants::AL_STATE_INIT, "back to Init");

  // Configure valid SM0 & SM1 and transition Init -> PreOp.
  write_ecat_sm(hw, 0U, cfg.mbx0_address, cfg.mbx_size,
                constants::SM_CTRL_DIR_WRITE | constants::SM_CTRL_MODE_MAILBOX,
                constants::SM_ACTIVATE_ENABLE);
  write_ecat_sm(hw, 1U, cfg.mbx1_address, cfg.mbx_size,
                constants::SM_CTRL_DIR_READ | constants::SM_CTRL_MODE_MAILBOX,
                constants::SM_ACTIVATE_ENABLE);
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_PREOP)
                 .has_value(), "req preop");
  check_true(app.step_freerun().has_value(), "step preop");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U) ==
                 constants::AL_STATE_PREOP, "in PreOp");

  // Configure SM2 & SM3 and transition PreOp -> SafeOp -> Op.
  write_ecat_sm(hw, 2U, cfg.sm2_address, 1U, constants::SM_CTRL_DIR_WRITE,
                constants::SM_ACTIVATE_ENABLE);
  write_ecat_sm(hw, 3U, cfg.sm3_address, 1U, constants::SM_CTRL_DIR_READ,
                constants::SM_ACTIVATE_ENABLE);
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_SAFEOP)
                 .has_value(), "req safeop");
  check_true(app.step_freerun().has_value(), "step safeop");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U) ==
                 constants::AL_STATE_SAFEOP, "in SafeOp");

  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_OP)
                 .has_value(), "req op");
  check_true(app.step_freerun().has_value(), "step op");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U) ==
                 constants::AL_STATE_OP, "in Op");

  // Simulate process-data watchdog expiry (`0x0440` == 0) -> SafeOp + Error + callback.
  check_true(hw.ecat_write_u16(constants::ESCREG_WDSTATUS, 0x0000U).has_value(), "wd 0");
  check_true(app.step_freerun().has_value(), "step wd");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALSTATUS).value_or(0U) ==
                 (constants::AL_STATE_SAFEOP | constants::AL_STATE_ERROR_BIT),
             "safeop+err on wd");
  check_true(hw.ecat_read_u16(constants::ESCREG_ALERROR).value_or(0U) ==
                 constants::ALERR_WATCHDOG, "alerr wd");
  check_true(hooks.safe_output_calls >= 1U, "safe output hook called");
}

} // namespace

/// @brief Entry point for ESM and EEPROM unit test suite.
int main() {
  test_eeprom_emulation();
  test_esm_and_watchdog();
  std::cout << "resoes ESM and EEPROM unit tests passed." << std::endl;
  return 0;
}
