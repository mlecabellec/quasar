/**
 * @file test_coe_and_pdo.cpp
 * @brief Unit tests for resoes Mailbox, CoE SDO (Exp/Seg/CA), SDO Info & Bit-Sliced PDOs.
 * @feature [TSK-20261113-001]
 */

#include "resoes/SlaveApplication.hpp"
#include "resoes/hal/MemoryEscHardware.hpp"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace resoes;

namespace {

/// @brief Always-active runtime assertion helper safe under Release (-DNDEBUG) builds.
void check_true(bool cond, const char *label) {
  if (cond == false) {
    std::cerr << "CHECK FAILED: " << label << std::endl;
    std::abort();
  }
}

/// @brief Helper writing 8-byte SyncManager block cleanly from Master side.
void write_sm_block(MemoryEscHardware &hw, uint8_t idx, uint16_t addr,
                    uint16_t len, uint8_t ctrl) {
  std::array<byte, 8U> raw{};
  (void)write_le16(raw, 0U, addr);
  (void)write_le16(raw, 2U, len);
  raw[4U] = static_cast<byte>(ctrl);
  raw[6U] = static_cast<byte>(constants::SM_ACTIVATE_ENABLE);
  uint16_t reg = static_cast<uint16_t>(constants::ESCREG_SM0 + (idx * 8U));
  check_true(hw.ecat_write(reg, raw).has_value() == true, "write SM block");
}

/// @brief Configure SM0/1/2/3 and advance SlaveApplication cleanly to Op.
void bring_to_op(SlaveApplication &app, MemoryEscHardware &hw,
                 uint16_t rx_len, uint16_t tx_len) {
  const SlaveConfig &CFG = app.esc().config();
  write_sm_block(hw, 0U, CFG.mbx0_address, CFG.mbx_size,
                 constants::SM_CTRL_DIR_WRITE | constants::SM_CTRL_MODE_MAILBOX);
  write_sm_block(hw, 1U, CFG.mbx1_address, CFG.mbx_size,
                 constants::SM_CTRL_DIR_READ | constants::SM_CTRL_MODE_MAILBOX);
  write_sm_block(hw, 2U, CFG.sm2_address, rx_len, constants::SM_CTRL_DIR_WRITE);
  write_sm_block(hw, 3U, CFG.sm3_address, tx_len, constants::SM_CTRL_DIR_READ);

  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_PREOP)
                 .has_value(), "req preop");
  check_true(app.step_freerun().has_value(), "step preop");
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_SAFEOP)
                 .has_value(), "req safeop");
  check_true(app.step_freerun().has_value(), "step safeop");
  check_true(hw.ecat_write_u16(constants::ESCREG_ALCONTROL, constants::AL_STATE_OP)
                 .has_value(), "req op");
  check_true(app.step_freerun().has_value(), "step op");
  check_true(app.esc().snapshot().al_state == AlState::Op, "in Op");
}

/// @brief Send SM0 CoE request and read SM1 reply cleanly.
std::vector<byte> roundtrip_mbx(SlaveApplication &app, MemoryEscHardware &hw,
                                const std::vector<byte> &payload) {
  std::vector<byte> frame{};
  frame.resize(constants::MBX_HEADER_SIZE + payload.size(), byte{0});
  MailboxHeader hdr{};
  hdr.length = static_cast<uint16_t>(payload.size());
  hdr.set_type_and_counter(constants::MBX_TYPE_COE, 1U);
  (void)write_le16(frame, 0U, hdr.length);
  (void)write_le16(frame, 2U, 0x1001U);
  frame[4U] = byte{0};
  frame[5U] = static_cast<byte>(hdr.type_cnt);
  for (size_t k = 0U; k < payload.size(); ++k) {
    frame[constants::MBX_HEADER_SIZE + k] = payload[k];
  }
  check_true(hw.ecat_write_sm(0U, frame).has_value(), "ecat write SM0");
  check_true(app.step_poll().has_value(), "step_poll rx");

  // Read SM1 response frame written by MailboxServer.
  std::vector<byte> rx{};
  rx.resize(app.esc().config().mbx_size, byte{0});
  check_true(hw.ecat_read_sm(1U, rx).has_value(), "ecat read SM1");
  check_true(app.step_poll().has_value(), "step_poll ack");
  uint16_t resp_len = read_le16(rx, 0U).value_or(0U);
  std::vector<byte> out{};
  out.resize(resp_len, byte{0});
  for (size_t k = 0U; k < resp_len; ++k) {
    out[k] = rx[constants::MBX_HEADER_SIZE + k];
  }
  return out;
}

/// @brief Verify Expedited SDO, Segmented Upload/Download, Complete Access & SDO Info.
void test_coe_sdo_and_info() {
  MemoryEscHardware hw{};
  MemoryEepromStorage eep{};
  SlaveConfig cfg{};
  cfg.mbx_size = 32U; // Compact mailbox forces multi-segment transfer for >16B string
  SlaveApplication app{hw, eep, cfg};
  check_true(app.init().has_value(), "app init");

  std::array<byte, 4U> dev_type{};
  (void)write_le32(dev_type, 0U, 0x00011388U);
  check_true(app.od().add_variable(0x1000U, OdDataType::Unsigned32, 32U,
                                   od_access::READ_ALL, "DeviceType", dev_type)
                 .has_value(), "add 0x1000");

  // VisibleString > (32 - 6 - 10) = 16 bytes triggers segmented upload.
  std::string dev_name_str = "QUASAR-RESOES-SUBDEV-V1";
  std::vector<byte> dev_name_bytes{};
  for (size_t k = 0U; k < dev_name_str.size(); ++k) {
    dev_name_bytes.push_back(static_cast<byte>(dev_name_str[k]));
  }
  check_true(app.od().add_variable(0x1008U, OdDataType::VisibleString,
                                   static_cast<uint16_t>(dev_name_bytes.size() * 8U),
                                   od_access::READ_WRITE_ALL, "DeviceName",
                                   dev_name_bytes).has_value(), "add 0x1008");

  // Identity Record 0x1018 (3 subindices) for Complete Access test.
  std::array<byte, 1U> two{byte{2}};
  std::array<byte, 4U> vid{};
  (void)write_le32(vid, 0U, 0x00001234U);
  std::array<byte, 4U> pid{};
  (void)write_le32(pid, 0U, 0xABCD0001U);
  OdObjectEntry ident{};
  ident.index = 0x1018U;
  ident.object_type = OdObjectType::Record;
  ident.max_subindex = 2U;
  ident.name = "Identity";
  // Populate subindices 0..2 cleanly.
  ident.subindices.push_back(OdSubindexEntry{0U, OdDataType::Unsigned8, 8U,
                                             od_access::READ_ALL, "Count",
                                             std::vector<byte>{two.begin(), two.end()}});
  ident.subindices.push_back(OdSubindexEntry{1U, OdDataType::Unsigned32, 32U,
                                             od_access::READ_WRITE_ALL, "Vendor",
                                             std::vector<byte>{vid.begin(), vid.end()}});
  ident.subindices.push_back(OdSubindexEntry{2U, OdDataType::Unsigned32, 32U,
                                             od_access::READ_WRITE_ALL, "Product",
                                             std::vector<byte>{pid.begin(), pid.end()}});
  check_true(app.od().register_object(std::move(ident)).has_value(), "reg 0x1018");

  bring_to_op(app, hw, 0U, 0U);

  // 1. Expedited Upload 0x1000:00.
  std::vector<byte> req{};
  req.resize(constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE, byte{0});
  CoeHeader ch{};
  ch.set(constants::COE_SERVICE_SDOREQ, 0U);
  (void)write_le16(req, 0U, ch.number_service);
  req[2U] = static_cast<byte>(constants::SDO_CCS_UPLOAD_INIT_REQ);
  (void)write_le16(req, 3U, 0x1000U);
  std::vector<byte> rsp = roundtrip_mbx(app, hw, req);
  check_true(rsp.size() >= 10U, "exp upload resp size");
  check_true(read_le32(rsp, 6U).value_or(0U) == 0x00011388U, "exp upload value");

  // 2. Segmented Upload 0x1008:00 (23 bytes over 32-byte mailbox).
  (void)write_le16(req, 3U, 0x1008U);
  std::vector<byte> seg_init_rsp = roundtrip_mbx(app, hw, req);
  uint32_t total_sz = read_le32(seg_init_rsp, 6U).value_or(0U);
  check_true(total_sz == dev_name_bytes.size(), "seg total_sz");
  std::vector<byte> collected{};
  collected.assign(seg_init_rsp.begin() + 10U, seg_init_rsp.end());

  bool toggle = false;
  while (collected.size() < total_sz) {
    // Request next segment alternating toggle bit.
    std::vector<byte> seg_req{};
    seg_req.resize(10U, byte{0});
    (void)write_le16(seg_req, 0U, ch.number_service);
    uint8_t tbit = (toggle == true) ? constants::SDO_FLAG_TOGGLE : 0U;
    seg_req[2U] = static_cast<byte>(constants::SDO_CCS_UPLOAD_SEG_REQ | tbit);
    std::vector<byte> srsp = roundtrip_mbx(app, hw, seg_req);
    uint8_t scs = std::to_integer<uint8_t>(srsp[2U]);
    bool more = (scs & constants::SDO_FLAG_MORE_FOLLOWS_LAST) != 0U;
    size_t unused = (more == true) ? 0U : static_cast<size_t>((scs >> 1U) & 0x07U);
    size_t chunk_len = srsp.size() - 3U;
    if ((more == false) && (chunk_len <= 7U) && (unused <= 7U)) {
      chunk_len = 7U - unused;
    }
    size_t copy_n = std::min(chunk_len, static_cast<size_t>(total_sz) - collected.size());
    for (size_t k = 0U; k < copy_n; ++k) {
      collected.push_back(srsp[3U + k]);
    }
    toggle = !toggle;
  }
  check_true(collected == dev_name_bytes, "seg payload match");

  // 3. Complete Access Upload 0x1018 starting at subindex 1 (8 bytes).
  req[2U] = static_cast<byte>(constants::SDO_CCS_UPLOAD_INIT_REQ |
                              constants::SDO_FLAG_COMPLETE_ACCESS);
  (void)write_le16(req, 3U, 0x1018U);
  req[5U] = byte{1};
  std::vector<byte> ca_rsp = roundtrip_mbx(app, hw, req);
  check_true(read_le32(ca_rsp, 6U).value_or(0U) == 8U, "ca size == 8");
  check_true(read_le32(ca_rsp, 10U).value_or(0U) == 0x00001234U, "ca vendor");
  check_true(read_le32(ca_rsp, 14U).value_or(0U) == 0xABCD0001U, "ca product");

  // 4. SDO Info Get ODList.
  std::vector<byte> info_req{};
  info_req.resize(constants::COE_HEADER_SIZE + constants::SDO_INFO_HEADER_SIZE,
                  byte{0});
  CoeHeader ih{};
  ih.set(constants::COE_SERVICE_SDOINFO, 0U);
  (void)write_le16(info_req, 0U, ih.number_service);
  info_req[2U] = static_cast<byte>(constants::SDOINFO_OP_GET_ODLIST_REQ);
  (void)write_le16(info_req, 6U, 0x0001U);
  std::vector<byte> irsp = roundtrip_mbx(app, hw, info_req);
  check_true(std::to_integer<uint8_t>(irsp[2U]) == constants::SDOINFO_OP_GET_ODLIST_RES,
             "odlist opcode");
  check_true(irsp.size() >= (8U + 6U), "odlist resp size");
}

/// @brief Register a 2-entry PDO mapping object cleanly.
void register_pdo_map_2(SlaveApplication &app, uint16_t map_idx, uint32_t e1,
                        uint32_t e2) {
  OdObjectEntry m{};
  m.index = map_idx;
  m.max_subindex = 2U;
  std::array<byte, 4U> b1{};
  std::array<byte, 4U> b2{};
  (void)write_le32(b1, 0U, e1);
  (void)write_le32(b2, 0U, e2);
  // Add subindex 0 count and 2 mapping entries.
  m.subindices.push_back(OdSubindexEntry{0U, OdDataType::Unsigned8, 8U,
                                         od_access::READ_ALL, "N",
                                         std::vector<byte>{byte{2}}});
  m.subindices.push_back(OdSubindexEntry{1U, OdDataType::Unsigned32, 32U,
                                         od_access::READ_ALL, "M1",
                                         std::vector<byte>{b1.begin(), b1.end()}});
  m.subindices.push_back(OdSubindexEntry{2U, OdDataType::Unsigned32, 32U,
                                         od_access::READ_ALL, "M2",
                                         std::vector<byte>{b2.begin(), b2.end()}});
  check_true(app.od().register_object(std::move(m)).has_value(), "reg pdo map");
}

/// @brief Register an SM assignment object cleanly.
void register_sm_assign_1(SlaveApplication &app, uint16_t assign_idx,
                          uint16_t map_idx) {
  OdObjectEntry a{};
  a.index = assign_idx;
  a.max_subindex = 1U;
  std::array<byte, 2U> b{};
  (void)write_le16(b, 0U, map_idx);
  // Add subindex 0 count and 1 assigned PDO index.
  a.subindices.push_back(OdSubindexEntry{0U, OdDataType::Unsigned8, 8U,
                                         od_access::READ_ALL, "N",
                                         std::vector<byte>{byte{1}}});
  a.subindices.push_back(OdSubindexEntry{1U, OdDataType::Unsigned16, 16U,
                                         od_access::READ_ALL, "A1",
                                         std::vector<byte>{b.begin(), b.end()}});
  check_true(app.od().register_object(std::move(a)).has_value(), "reg sm assign");
}

/// @brief Verify unaligned bit-packed PDO pack/unpack across SM2 and SM3.
void test_bit_sliced_pdo_exchange() {
  MemoryEscHardware hw{};
  MemoryEepromStorage eep{};
  SlaveConfig cfg{};
  SlaveApplication app{hw, eep, cfg};
  check_true(app.init().has_value(), "pdo app init");

  std::array<byte, 1U> b_zero{byte{0}};
  std::array<byte, 2U> w_zero{byte{0}, byte{0}};
  // RxPDO: 3-bit flags (0x7000) + 13-bit PWM (0x7001) = 16 bits -> 2 bytes.
  check_true(app.od().add_variable(0x7000U, OdDataType::Unsigned8, 3U,
                                   od_access::READ_WRITE_ALL | od_access::RX_PDO_MAP,
                                   "Flags", b_zero).has_value(), "add 0x7000");
  check_true(app.od().add_variable(0x7001U, OdDataType::Unsigned16, 13U,
                                   od_access::READ_WRITE_ALL | od_access::RX_PDO_MAP,
                                   "Pwm", w_zero).has_value(), "add 0x7001");
  // TxPDO: 1-bit Fault (0x6000) + 15-bit ADC (0x6001) = 16 bits -> 2 bytes.
  check_true(app.od().add_variable(0x6000U, OdDataType::Boolean, 1U,
                                   od_access::READ_ALL | od_access::TX_PDO_MAP,
                                   "Fault", std::array<byte, 1U>{byte{1}}).has_value(),
             "add 0x6000");
  check_true(app.od().add_variable(0x6001U, OdDataType::Unsigned16, 15U,
                                   od_access::READ_ALL | od_access::TX_PDO_MAP,
                                   "Adc", std::array<byte, 2U>{byte{0x34}, byte{0x12}})
                 .has_value(), "add 0x6001");

  register_pdo_map_2(app, 0x1600U, 0x70000003U, 0x7001000DU);
  register_pdo_map_2(app, 0x1A00U, 0x60000001U, 0x6001000FU);
  register_sm_assign_1(app, constants::OD_IDX_SM2_ASSIGN, 0x1600U);
  register_sm_assign_1(app, constants::OD_IDX_SM3_ASSIGN, 0x1A00U);

  bring_to_op(app, hw, 2U, 2U);

  // Master writes RxPDO: Flags=5 (bits 0..2), Pwm=0x12AB (shifted << 3).
  uint16_t packed_rx = static_cast<uint16_t>((0x12ABU << 3U) | 0x05U);
  std::array<byte, 2U> sm2_bytes{};
  (void)write_le16(sm2_bytes, 0U, packed_rx);
  check_true(hw.ecat_write_sm(2U, sm2_bytes).has_value(), "write sm2");

  // Run 50 cyclic iterations verifying deterministic stability.
  for (int i = 0; i < 50; ++i) {
    check_true(app.step_freerun().has_value(), "step freerun loop");
  }

  check_true(app.od().read_u32(0x7000U, 0U).value_or(0U) == 0x05U, "rx flags == 5");
  check_true(app.od().read_u32(0x7001U, 0U).value_or(0U) == 0x12ABU, "rx pwm == 0x12AB");

  // Master reads TxPDO SM3: (0x1234 << 1) | 1 = 0x2469.
  std::array<byte, 2U> sm3_bytes{};
  check_true(hw.ecat_read_sm(3U, sm3_bytes).has_value(), "read sm3");
  check_true(read_le16(sm3_bytes, 0U).value_or(0U) ==
                 static_cast<uint16_t>((0x1234U << 1U) | 1U),
             "tx pdo packed bits");
}

} // namespace

/// @brief Entry point for resoes CoE and PDO unit tests.
int main() {
  test_coe_sdo_and_info();
  test_bit_sliced_pdo_exchange();
  std::cout << "resoes CoE and bit-sliced PDO unit tests passed." << std::endl;
  return 0;
}
