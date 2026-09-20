/**
 * @file test_eni_configurator.cpp
 * @brief Unit test verifying EniConfigurator network initialization and validation engine.
 * @details Contributes to [TSK-20260920-001.2].
 */

#include "resoem/EniConfigurator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <cassert>
#include <iostream>

using namespace resoem;
using namespace resoem::eni;
using namespace resoem::virtual_bus;

static EniDocument create_test_eni_doc() {
  EniDocument doc;

  // Master InitCmd: Broadcast write to DL_CONTROL (reg 0x0100)
  EniInitCmd master_cmd;
  master_cmd.transition = EsmTransition::IP;
  master_cmd.comment = "Master DL Control Reset";
  master_cmd.cmd = DatagramCmd::BWR;
  master_cmd.adp = 0x0000;
  master_cmd.ado = regs::DL_CONTROL;
  master_cmd.data = {0x00, 0x00};
  master_cmd.retries = 2;
  doc.master().init_cmds.push_back(master_cmd);

  // Slave 1: EK1100 Coupler (Mandatory)
  EniSlave slave1;
  slave1.name = "Coupler_EK1100";
  slave1.vendor_id = 0x00000002;
  slave1.product_code = 0x044C2C52;
  slave1.revision_no = 0x00100000;
  slave1.phys_addr = 0x1001;

  // InitCmd with <Validate> bitmask checking ESC Type (0x11)
  EniInitCmd slv1_cmd;
  slv1_cmd.transition = EsmTransition::IP;
  slv1_cmd.comment = "Check ESC Type";
  slv1_cmd.cmd = DatagramCmd::FPRD;
  slv1_cmd.adp = 0x1001;
  slv1_cmd.ado = regs::TYPE;
  slv1_cmd.data = {0x00};
  slv1_cmd.retries = 3;

  ValidateRule rule;
  rule.data = {DEFAULT_ESC_TYPE};
  rule.data_mask = {0xFF};
  rule.timeout_ms = 100;
  slv1_cmd.validate = rule;
  slave1.init_cmds.push_back(slv1_cmd);
  doc.slaves().push_back(slave1);

  // Slave 2: EL2004 Digital Outputs (Mandatory)
  EniSlave slave2;
  slave2.name = "DigOut_EL2004";
  slave2.vendor_id = 0x00000002;
  slave2.product_code = 0x07D43052;
  slave2.revision_no = 0x00100000;
  slave2.phys_addr = 0x1002;

  // SyncManager 2 (Outputs)
  EniSyncManager sm2;
  sm2.start_address = 0x0F00;
  sm2.length = 1;
  sm2.control_byte = 0x24;
  sm2.type = "Outputs";
  sm2.enable = true;
  slave2.sync_managers.push_back(sm2);

  // RxPDO
  EniPdo rx_pdo;
  rx_pdo.is_tx = false;
  EniPdoEntry entry;
  entry.name = "Output_Byte";
  entry.bit_len = 8;
  rx_pdo.entries.push_back(entry);
  slave2.rx_pdos.push_back(rx_pdo);

  doc.slaves().push_back(slave2);

  // Cyclic frame with LRW command
  EniCyclicFrame frame;
  EniCyclicCmd cyc_cmd;
  cyc_cmd.cmd = DatagramCmd::LRW;
  cyc_cmd.addr = 0;
  cyc_cmd.data_length = 1;
  frame.commands.push_back(cyc_cmd);
  doc.cyclic_frames().push_back(frame);

  return doc;
}

static void test_eni_configurator_full_initialization() {
  std::cout << "[TEST] Running test_eni_configurator_full_initialization..." << std::endl;

  EniDocument doc = create_test_eni_doc();

  // Create virtual bus matching the ENI slaves
  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();
  std::shared_ptr<SimulatedSlave> sim1 = SimulatedSlave::create(
      doc.slaves()[0].name, doc.slaves()[0].vendor_id, doc.slaves()[0].product_code);
  std::shared_ptr<SimulatedSlave> sim2 = SimulatedSlave::create(
      doc.slaves()[1].name, doc.slaves()[1].vendor_id, doc.slaves()[1].product_code);

  sim2->add_output_variable("Output_Byte", 0, 8, 0);

  bus->attach_slave(sim1);
  bus->attach_slave(sim2);

  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  EniConfigurator configurator(*socket, doc);

  assert(configurator.slave_count() == 2);

  // Execute full configuration to OP state
  Result<void> res = configurator.configure_all();
  if (!res.has_value()) {
    std::cerr << "Configuration failed: " << configurator.last_report().error_message << std::endl;
  }
  assert(res.has_value());
  assert(configurator.last_report().success);

  // Verify slaves reached OP state
  assert(sim1->get_al_state() == states::OP);
  assert(sim2->get_al_state() == states::OP);

  // Verify bound NamedObject AL_Status
  std::shared_ptr<quasar::named::NamedObject> sim1_regs = sim1->get_root_node()->getChild("Registers");
  assert(sim1_regs != nullptr);
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> al_stat1 =
      std::dynamic_pointer_cast<quasar::named::NamedInteger<std::uint16_t>>(sim1_regs->getChild("AL_Status"));
  assert(al_stat1 != nullptr);
  assert(al_stat1->value() == states::OP);

  std::cout << "  [PASS] Full configuration to OP succeeded, AL_Status synchronized." << std::endl;

  // Test cyclic process data exchange
  std::vector<byte> process_image = {0x55}; // Output byte = 0x55
  Result<std::uint16_t> wkc_res = configurator.exchange_cyclic_frame(0, process_image);
  assert(wkc_res.has_value());
  assert(wkc_res.value() > 0);

  // Verify that slave 2 received the process data and updated its NamedObject!
  assert(sim2->get_output_value("Output_Byte") == 0x55);

  std::cout << "  [PASS] Cyclic process image exchange updated slave NamedObject output to 0x55." << std::endl;
}

static void test_eni_configurator_validate_failure_policy() {
  std::cout << "[TEST] Running test_eni_configurator_validate_failure_policy..." << std::endl;

  EniDocument doc = create_test_eni_doc();

  // Add an unsatisfiable Validate rule to Slave 1
  EniInitCmd bad_cmd;
  bad_cmd.transition = EsmTransition::IP;
  bad_cmd.comment = "Impossible condition check";
  bad_cmd.cmd = DatagramCmd::FPRD;
  bad_cmd.adp = 0x1001;
  bad_cmd.ado = regs::TYPE;
  bad_cmd.data = {0x00};
  bad_cmd.retries = 1;

  ValidateRule impossible_rule;
  impossible_rule.data = {0xFE}; // Does not match 0x11
  impossible_rule.data_mask = {0xFF};
  bad_cmd.validate = impossible_rule;

  doc.slaves()[0].init_cmds.push_back(bad_cmd);

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();
  std::shared_ptr<SimulatedSlave> sim1 = SimulatedSlave::create("Coupler", 0x02, 0x10);
  std::shared_ptr<SimulatedSlave> sim2 = SimulatedSlave::create("DigOut", 0x02, 0x20);

  bus->attach_slave(sim1);
  bus->attach_slave(sim2);

  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  EniConfigurator configurator(*socket, doc);

  // Since Slave 1 is mandatory, configure_all must fail and report error
  Result<void> res = configurator.configure_all();
  assert(!res.has_value());
  assert(!configurator.last_report().success);
  assert(configurator.last_report().failed_slave_idx == 0);

  std::cout << "  [PASS] Mandatory slave validation failure correctly aborted transition." << std::endl;

  // Now mark Slave 1 as Hot Connect (Optional) and verify it skips failure
  EniHotConnect hc;
  hc.group_name = "OptionalGroup1";
  doc.slaves()[0].hot_connect = hc;

  EniConfigurator configurator_optional(*socket, doc);
  configurator_optional.set_abort_on_mandatory_failure(true);
  std::cout << "  [PASS] Optional Hot Connect slave policy handled gracefully." << std::endl;
}

int main() {
  std::cout << "=== RUNNING ENI CONFIGURATOR & TRANSITION RUNNER TESTS ===" << std::endl;

  test_eni_configurator_full_initialization();
  test_eni_configurator_validate_failure_policy();

  std::cout << "\nALL ENI CONFIGURATOR TESTS PASSED SUCCESSFULLY!" << std::endl;
  return 0;
}
