/**
 * @file test_virtual_bus.cpp
 * @brief Unit test verifying SimulatedSlave, VirtualEtherCATBus, and StubRawSocket.
 * @details Contributes to [TSK-20260920-001.11] and [TSK-20260920-001.12].
 */

#include "resoem/Enumerator.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <cassert>
#include <iostream>

using namespace resoem;
using namespace resoem::virtual_bus;

static void test_simulated_slave_named_hierarchy() {
  std::cout << "[TEST] Running test_simulated_slave_named_hierarchy..." << std::endl;

  std::shared_ptr<SimulatedSlave> slave = SimulatedSlave::create(
      "TestCoupler", 0x00000002, 0x044C2C52, 0x00100000, 0x00000001);

  assert(slave != nullptr);
  std::shared_ptr<quasar::named::NamedObject> root = slave->get_root_node();
  assert(root != nullptr);
  assert(root->getName() == "TestCoupler");

  // Verify dedicated subtrees: Registers/, Inputs/, Outputs/
  std::shared_ptr<quasar::named::NamedObject> regs_node = root->getChild("Registers");
  assert(regs_node != nullptr);

  std::shared_ptr<quasar::named::NamedObject> inputs_node = root->getChild("Inputs");
  assert(inputs_node != nullptr);

  std::shared_ptr<quasar::named::NamedObject> outputs_node = root->getChild("Outputs");
  assert(outputs_node != nullptr);

  // Check initial AL status = 1 (INIT)
  std::shared_ptr<quasar::named::NamedObject> al_status_obj = regs_node->getChild("AL_Status");
  assert(al_status_obj != nullptr);
  std::shared_ptr<quasar::named::NamedInteger<std::uint16_t>> al_status_node =
      std::dynamic_pointer_cast<quasar::named::NamedInteger<std::uint16_t>>(al_status_obj);
  assert(al_status_node != nullptr);
  assert(al_status_node->getValue() == states::INIT);

  // Add TxPDO (Input) and RxPDO (Output) variables
  slave->add_input_variable("SensorVal", 0, 16, 42);
  slave->add_output_variable("ActuatorCmd", 0, 16, 100);

  assert(slave->get_input_value("SensorVal") == 42);
  assert(slave->get_output_value("ActuatorCmd") == 100);

  // Update input value and verify
  slave->set_input_value("SensorVal", 84);
  assert(slave->get_input_value("SensorVal") == 84);

  std::cout << "  [PASS] SimulatedSlave NamedObject hierarchy verified successfully." << std::endl;
}

static void test_virtual_bus_enumeration() {
  std::cout << "[TEST] Running test_virtual_bus_enumeration..." << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();
  assert(bus != nullptr);

  // Create 3 simulated slaves: Coupler, 4-ch Digital Output, 4-ch Digital Input
  std::shared_ptr<SimulatedSlave> slave1 = SimulatedSlave::create("Coupler_EK1100", 0x00000002, 0x044C2C52);
  std::shared_ptr<SimulatedSlave> slave2 = SimulatedSlave::create("DigOut_EL2004", 0x00000002, 0x07D43052);
  std::shared_ptr<SimulatedSlave> slave3 = SimulatedSlave::create("DigIn_EL1004", 0x00000002, 0x03EC3052);

  slave2->add_output_variable("Channel1_Out", 0, 1, 0);
  slave2->add_output_variable("Channel2_Out", 1, 1, 0);
  slave3->add_input_variable("Channel1_In", 0, 1, 1);

  bus->attach_slave(slave1);
  bus->attach_slave(slave2);
  bus->attach_slave(slave3);

  assert(bus->slave_count() == 3);

  // Create virtual socket and enumerator
  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  assert(socket != nullptr);
  assert(socket->interface_name() == "virtual_eth0");

  Enumerator enumerator(*socket);

  // Run enumeration completely root-free and hardware-free!
  Result<std::size_t> count_res = enumerator.enumerate();
  assert(count_res.has_value());
  assert(count_res.value() == 3);

  const std::vector<SlaveInfo> &slaves = enumerator.slaves();
  assert(slaves.size() == 3);
  assert(slaves[0].configured_address == 0x1001);
  assert(slaves[1].configured_address == 0x1002);
  assert(slaves[2].configured_address == 0x1003);

  // Verify vendor IDs read through simulated EEPROM
  assert(slaves[0].vendor_id == 0x00000002);
  assert(slaves[1].vendor_id == 0x00000002);
  assert(slaves[2].vendor_id == 0x00000002);

  // Verify product codes
  assert(slaves[0].product_code == 0x044C2C52);
  assert(slaves[1].product_code == 0x07D43052);
  assert(slaves[2].product_code == 0x03EC3052);

  std::cout << "  [PASS] Enumeration over virtual bus found 3 slaves with correct SII data." << std::endl;

  // Test state transitions
  Result<> preop_res = enumerator.request_state_all(states::PRE_OP);
  assert(preop_res.has_value());
  assert(slave1->get_al_state() == states::PRE_OP);
  assert(slave2->get_al_state() == states::PRE_OP);
  assert(slave3->get_al_state() == states::PRE_OP);

  Result<> safeop_res = enumerator.request_state_all(states::SAFE_OP);
  assert(safeop_res.has_value());
  assert(slave1->get_al_state() == states::SAFE_OP);

  Result<> op_res = enumerator.request_state_all(states::OP);
  assert(op_res.has_value());
  assert(slave1->get_al_state() == states::OP);

  std::cout << "  [PASS] State transitions (PRE_OP -> SAFE_OP -> OP) succeeded." << std::endl;
}

static void test_virtual_bus_cable_fault_injection() {
  std::cout << "[TEST] Running test_virtual_bus_cable_fault_injection..." << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create("Slave1", 0x02, 0x10);
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create("Slave2", 0x02, 0x20);
  std::shared_ptr<SimulatedSlave> s3 = SimulatedSlave::create("Slave3", 0x02, 0x30);

  bus->attach_slave(s1);
  bus->attach_slave(s2);
  bus->attach_slave(s3);

  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);
  Enumerator enumerator(*socket);

  Result<std::size_t> full_count = enumerator.enumerate();
  assert(full_count.has_value());
  assert(full_count.value() == 3);

  // Inject a cable break between slave 0 and slave 1
  bus->break_cable_after(0);
  assert(bus->is_cable_broken());

  // Enumerate again: primary frames only reach slave 0
  Result<std::size_t> broken_count = enumerator.enumerate();
  assert(broken_count.has_value());
  assert(broken_count.value() == 1);

  // Restore cable
  bus->restore_cable();
  assert(!bus->is_cable_broken());

  Result<std::size_t> restored_count = enumerator.enumerate();
  assert(restored_count.has_value());
  assert(restored_count.value() == 3);

  std::cout << "  [PASS] Cable fault injection and healing verified." << std::endl;
}

int main() {
  std::cout << "=== RUNNING VIRTUAL ETHERCAT BUS & HARDWARE STUB TESTS ===" << std::endl;

  test_simulated_slave_named_hierarchy();
  test_virtual_bus_enumeration();
  test_virtual_bus_cable_fault_injection();

  std::cout << "\nALL VIRTUAL BUS & HARDWARE STUB TESTS PASSED SUCCESSFULLY!" << std::endl;
  return 0;
}
