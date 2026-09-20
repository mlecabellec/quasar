/**
 * @file test_hot_connect.cpp
 * @brief Unit test verifying EtherCAT Hot Connect dynamic discovery, ESM stepping, and policy enforcement.
 * @details Contributes to [TSK-20260920-001.8], [TSK-20260920-001.9], and [FE-0040.8].
 */

#include "resoem/EniConfigurator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/HotConnectConstants.hpp"
#include "resoem/HotConnectManager.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"

// Standard library includes
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

using namespace resoem;
using namespace resoem::eni;
using namespace resoem::virtual_bus;
using namespace resoem::hot_connect;

namespace {

constexpr std::uint16_t SLAVE_0_ADDR = 0x1001;
constexpr std::uint16_t SLAVE_1_ADDR = 0x1002;
constexpr std::uint16_t SLAVE_2_ADDR = 0x1003;
constexpr std::uint16_t TOOL_ARM_ALIAS = 0x2001;

EniDocument create_hot_connect_eni_doc() {
  EniDocument doc;

  // Master InitCmd
  EniInitCmd master_cmd;
  master_cmd.transition = EsmTransition::IP;
  master_cmd.cmd = DatagramCmd::BWR;
  master_cmd.adp = 0x0000;
  master_cmd.ado = regs::DL_CONTROL;
  master_cmd.data = {0x00, 0x00};
  doc.master().init_cmds.push_back(master_cmd);

  // Mandatory Slave 0: Coupler EK1100
  EniSlave slave0;
  slave0.name = "Coupler_EK1100";
  slave0.vendor_id = 0x00000002;
  slave0.product_code = 0x044C2C52;
  slave0.phys_addr = SLAVE_0_ADDR;
  doc.slaves().push_back(slave0);

  // Mandatory Slave 1: DigOut EL2004
  EniSlave slave1;
  slave1.name = "DigOut_EL2004";
  slave1.vendor_id = 0x00000002;
  slave1.product_code = 0x07D43052;
  slave1.phys_addr = SLAVE_1_ADDR;

  // Sync Manager 2 (Outputs)
  EniSyncManager sm2;
  sm2.type = "Outputs";
  sm2.start_address = 0x0F00;
  sm2.length = 1;
  sm2.control_byte = 0x24;
  sm2.enable = true;
  slave1.sync_managers.push_back(sm2);

  // Configure RxPDO for outputs
  EniPdo rx_pdo;
  rx_pdo.is_tx = false;
  EniPdoEntry rx_entry;
  rx_entry.name = "Out";
  rx_entry.bit_len = 8;
  rx_pdo.entries.push_back(rx_entry);
  slave1.rx_pdos.push_back(rx_pdo);
  doc.slaves().push_back(slave1);

  // Dynamic Hot Connect Slave 2: DigIn ToolArm (EL1004)
  EniSlave slave2;
  slave2.name = "ToolArm_EL1004";
  slave2.vendor_id = 0x00000002;
  slave2.product_code = 0x03EC3052;
  slave2.phys_addr = SLAVE_2_ADDR;

  EniHotConnect hc;
  hc.group_name = "ToolArm";
  hc.group_member_count = 1;

  EniInitCmd ident;
  ident.transition = EsmTransition::NONE;
  ident.cmd = DatagramCmd::FPRD;
  ident.adp = SLAVE_2_ADDR;
  ident.ado = regs::CONFIG_STATION_ALIAS;
  ident.data = {0x00, 0x00};

  ValidateRule rule;
  rule.data = {static_cast<std::uint8_t>(TOOL_ARM_ALIAS & 0xFF),
               static_cast<std::uint8_t>((TOOL_ARM_ALIAS >> 8) & 0xFF)};
  rule.data_mask = {0xFF, 0xFF};
  ident.validate = rule;
  hc.identify_cmds.push_back(ident);
  slave2.hot_connect = hc;

  EniSyncManager sm3;
  sm3.type = "Inputs";
  sm3.start_address = 0x0100;
  sm3.length = 1;
  sm3.control_byte = 0x20;
  sm3.enable = true;
  slave2.sync_managers.push_back(sm3);

  EniPdo tx_pdo;
  tx_pdo.is_tx = true;
  EniPdoEntry tx_entry;
  tx_entry.name = "In";
  tx_entry.bit_len = 8;
  tx_pdo.entries.push_back(tx_entry);
  slave2.tx_pdos.push_back(tx_pdo);
  doc.slaves().push_back(slave2);

  // Cyclic frame with 1 command: expected WKC = 2 when all operational
  EniCyclicFrame cyc;
  EniCyclicCmd cmd;
  cmd.cmd = DatagramCmd::LRW;
  cmd.addr = 0x00010000;
  cmd.data_length = 2;
  cmd.expected_wkc = 2;
  cyc.commands.push_back(cmd);
  doc.cyclic_frames().push_back(cyc);

  return doc;
}

} // anonymous namespace

static void test_hot_connect_disconnected_policies() {
  std::cout << "--- Running test_hot_connect_disconnected_policies ---" << std::endl;

  EniDocument doc = create_hot_connect_eni_doc();
  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();
  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  EniConfigurator configurator(*socket, doc);

  HotConnectManager hc_mgr(doc, *socket, configurator);
  hc_mgr.auto_discover_groups_from_eni();

  assert(hc_mgr.total_group_count() == 1);
  assert(hc_mgr.group_state("ToolArm") == HotConnectGroupState::DISCONNECTED);
  assert(hc_mgr.operational_group_count() == 0);

  // Base expected WKC without ToolArm is 1
  assert(hc_mgr.calculate_active_expected_wkc() == 1);

  // Configure ToolArm input slot in process image
  HotConnectGroupConfig grp_cfg;
  grp_cfg.name = "ToolArm";
  grp_cfg.slave_indices = {2};
  grp_cfg.station_alias = TOOL_ARM_ALIAS;
  grp_cfg.expected_wkc = 1;
  grp_cfg.input_process_offset = 1;
  grp_cfg.input_process_length = 1;
  grp_cfg.state = HotConnectGroupState::DISCONNECTED;
  grp_cfg.disconnected_policy = DisconnectedDataPolicy::ZERO_FILL;

  // Re-register with explicit offsets
  HotConnectManager hc_mgr2(doc, *socket, configurator);
  hc_mgr2.register_group(grp_cfg);

  // Process image with initial values: byte 0 is mandatory slave, byte 1 is ToolArm
  std::vector<byte> process_image{static_cast<byte>(0x55), static_cast<byte>(0xAA)};

  // 1. Test ZERO_FILL policy
  hc_mgr2.set_group_policy("ToolArm", DisconnectedDataPolicy::ZERO_FILL);
  hc_mgr2.apply_disconnected_data_policies(process_image);
  assert(process_image[0] == static_cast<byte>(0x55)); // untouched
  assert(process_image[1] == static_cast<byte>(0x00)); // zero-filled

  // 2. Test CUSTOM_FALLBACK policy
  std::array<byte, 1> fallback{static_cast<byte>(0x77)};
  hc_mgr2.set_group_policy("ToolArm", DisconnectedDataPolicy::CUSTOM_FALLBACK, fallback);
  process_image[1] = static_cast<byte>(0xFF);
  hc_mgr2.apply_disconnected_data_policies(process_image);
  assert(process_image[0] == static_cast<byte>(0x55));
  assert(process_image[1] == static_cast<byte>(0x77)); // custom fallback applied

  // 3. Test HOLD_LAST_STATE policy
  std::array<byte, 1> valid_data{static_cast<byte>(0x99)};
  hc_mgr2.record_valid_inputs("ToolArm", valid_data);
  hc_mgr2.set_group_policy("ToolArm", DisconnectedDataPolicy::HOLD_LAST_STATE);
  process_image[1] = static_cast<byte>(0x00);
  hc_mgr2.apply_disconnected_data_policies(process_image);
  assert(process_image[0] == static_cast<byte>(0x55));
  assert(process_image[1] == static_cast<byte>(0x99)); // held last valid state

  std::cout << "test_hot_connect_disconnected_policies passed successfully." << std::endl;
}

static void test_hot_connect_dynamic_plug_and_non_intrusive_stepping() {
  std::cout << "--- Running test_hot_connect_dynamic_plug_and_non_intrusive_stepping ---" << std::endl;

  EniDocument doc = create_hot_connect_eni_doc();
  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();

  // Attach only mandatory slaves initially
  std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create("Coupler_EK1100", 0x02, 0x044C2C52, 0x10, 1);
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create("DigOut_EL2004", 0x02, 0x07D43052, 0x10, 2);
  s0->set_configured_address(SLAVE_0_ADDR);
  s1->set_configured_address(SLAVE_1_ADDR);

  bus->attach_slave(s0);
  bus->attach_slave(s1);

  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  EniConfigurator configurator(*socket, doc);

  // Transition mandatory bus to OP
  Result<void> init_res = configurator.configure_all();
  assert(init_res.has_value());
  assert(s0->get_al_state() == states::OP);
  assert(s1->get_al_state() == states::OP);

  HotConnectManager hc_mgr(doc, *socket, configurator);
  hc_mgr.auto_discover_groups_from_eni();

  assert(hc_mgr.group_state("ToolArm") == HotConnectGroupState::DISCONNECTED);
  assert(hc_mgr.calculate_active_expected_wkc() == 1);

  // Scan cycle while unplugged: should remain DISCONNECTED
  hc_mgr.scan_cycle();
  assert(hc_mgr.group_state("ToolArm") == HotConnectGroupState::DISCONNECTED);

  // Dynamic Hot Plug: Plug in ToolArm (Slave 2) with Station Alias
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create("ToolArm_EL1004", 0x02, 0x03EC3052, 0x10, 3);
  s2->set_configured_address(SLAVE_2_ADDR);
  s2->set_station_alias(TOOL_ARM_ALIAS);
  bus->attach_slave(s2);

  // Run scan cycle: detects alias 0x2001 and steps s2 to OP non-intrusively
  hc_mgr.scan_cycle();

  assert(hc_mgr.group_state("ToolArm") == HotConnectGroupState::OPERATIONAL);
  assert(hc_mgr.operational_group_count() == 1);
  assert(s2->get_al_state() == states::OP);

  // Verify mandatory slaves remained in OP without reset
  assert(s0->get_al_state() == states::OP);
  assert(s1->get_al_state() == states::OP);

  // Expected WKC dynamically adjusted from 1 to 2
  assert(hc_mgr.calculate_active_expected_wkc() == 2);

  // Dynamic Disconnect: Detach Slave 2
  bus->clear_slaves();
  bus->attach_slave(s0);
  bus->attach_slave(s1);

  // Scan cycle detects disconnection
  hc_mgr.scan_cycle();
  hc_mgr.scan_cycle(); // second missed cycle confirms disconnect

  assert(hc_mgr.group_state("ToolArm") == HotConnectGroupState::DISCONNECTED);
  assert(hc_mgr.operational_group_count() == 0);
  assert(hc_mgr.calculate_active_expected_wkc() == 1);

  std::cout << "test_hot_connect_dynamic_plug_and_non_intrusive_stepping passed successfully." << std::endl;
}

int main() {
  std::cout << "=== RUNNING ETHERCAT HOT CONNECT TEST SUITE ===" << std::endl;

  test_hot_connect_disconnected_policies();
  test_hot_connect_dynamic_plug_and_non_intrusive_stepping();

  std::cout << "\nALL HOT CONNECT TESTS PASSED 100%!" << std::endl;
  return 0;
}
