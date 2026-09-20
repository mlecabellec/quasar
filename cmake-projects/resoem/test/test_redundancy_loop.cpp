/**
 * @file test_redundancy_loop.cpp
 * @brief Unit and integration tests for EtherCAT Cable Redundancy loop topology engine.
 * @details Contributes to [TSK-20260920-001.5], [TSK-20260920-001.6], and [TSK-20260920-001.7].
 */

#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/RedundancyConstants.hpp"
#include "resoem/RedundancyManager.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

using namespace resoem;
using namespace resoem::virtual_bus;
using namespace resoem::redundancy;

namespace {

constexpr std::uint16_t SLAVE_0_ADDR = 1001;
constexpr std::uint16_t SLAVE_1_ADDR = 1002;
constexpr std::uint16_t SLAVE_2_ADDR = 1003;

constexpr std::uint32_t LOGICAL_BASE_ADDR = 0x00010000;
constexpr std::uint16_t PHYSICAL_INPUT_ADDR = 0x0100;
constexpr std::uint16_t FMMU_INPUT_LEN = 2;
constexpr std::size_t TOTAL_PAYLOAD_LEN = 6;
constexpr std::uint16_t EXPECTED_FULL_WKC = 3;

/**
 * @brief Configure FMMU 0 on a simulated slave for logical input reading.
 * @param slave Slave instance.
 * @param logical_start Logical start address.
 * @param length Byte length.
 * @param phys_start Physical ESC memory address.
 */
void configure_slave_input_fmmu(SimulatedSlave &slave, std::uint32_t logical_start,
                                std::uint16_t length, std::uint16_t phys_start) {
  // FMMU 0 starts at 0x0600
  // 0x0600: Logical Start Addr (32-bit)
  slave.write_register(0x0600, static_cast<std::uint8_t>(logical_start & 0xFF));
  slave.write_register(0x0601, static_cast<std::uint8_t>((logical_start >> 8) & 0xFF));
  slave.write_register(0x0602, static_cast<std::uint8_t>((logical_start >> 16) & 0xFF));
  slave.write_register(0x0603, static_cast<std::uint8_t>((logical_start >> 24) & 0xFF));

  // 0x0604: Length (16-bit)
  slave.write_register(0x0604, static_cast<std::uint8_t>(length & 0xFF));
  slave.write_register(0x0605, static_cast<std::uint8_t>((length >> 8) & 0xFF));

  // 0x0606: Logical Start Bit = 0, Logical End Bit = 7
  slave.write_register(0x0606, 0);
  slave.write_register(0x0607, 7);

  // 0x0608: Physical Start Addr (16-bit)
  slave.write_register(0x0608, static_cast<std::uint8_t>(phys_start & 0xFF));
  slave.write_register(0x0609, static_cast<std::uint8_t>((phys_start >> 8) & 0xFF));

  // 0x060A: Physical Start Bit = 0
  slave.write_register(0x060A, 0);

  // 0x060B: Type (0x01 = Read)
  slave.write_register(0x060B, 0x01);

  // 0x060C: Enable = 1
  slave.write_register(0x060C, 0x01);
}

/**
 * @brief Helper to write two bytes of physical input data into a simulated slave.
 * @param slave Slave instance.
 * @param b0 First byte.
 * @param b1 Second byte.
 */
void write_slave_inputs(SimulatedSlave &slave, std::uint8_t b0, std::uint8_t b1) {
  std::array<byte, 2> buf{static_cast<byte>(b0), static_cast<byte>(b1)};
  slave.write_physical_ram(PHYSICAL_INPUT_ADDR, buf);
}

/**
 * @brief Helper to build a cyclic LRD frame buffer.
 * @return Raw Ethernet frame vector.
 */
std::vector<byte> build_cyclic_lrd_frame() {
  FrameBuilder builder;
  std::vector<byte> zeroes(TOTAL_PAYLOAD_LEN, static_cast<byte>(0x00));
  std::uint16_t log_lo = static_cast<std::uint16_t>(LOGICAL_BASE_ADDR & 0xFFFF);
  std::uint16_t log_hi = static_cast<std::uint16_t>((LOGICAL_BASE_ADDR >> 16) & 0xFFFF);

  builder.add_datagram(cmds::LRD, 0x01, log_lo, log_hi, zeroes);
  const std::span<const byte> TX_SPAN = builder.build();
  return std::vector<byte>(TX_SPAN.begin(), TX_SPAN.end());
}

} // anonymous namespace

/**
 * @brief Test normal circular loop operation without faults.
 */
void test_circular_normal() {
  std::cout << "--- Running test_circular_normal ---" << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();

  std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create("Slave0", 0, SLAVE_0_ADDR);
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create("Slave1", 1, SLAVE_1_ADDR);
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create("Slave2", 2, SLAVE_2_ADDR);

  bus->attach_slave(s0);
  bus->attach_slave(s1);
  bus->attach_slave(s2);

  configure_slave_input_fmmu(*s0, LOGICAL_BASE_ADDR + 0, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s1, LOGICAL_BASE_ADDR + 2, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s2, LOGICAL_BASE_ADDR + 4, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);

  write_slave_inputs(*s0, 0x11, 0x22);
  write_slave_inputs(*s1, 0x33, 0x44);
  write_slave_inputs(*s2, 0x55, 0x66);

  std::shared_ptr<StubRawSocket> sock = std::make_shared<StubRawSocket>(bus, true);
  RedundancyManager mgr(*sock);

  assert(mgr.state() == RedundancyState::CIRCULAR_NORMAL);
  assert(mgr.break_count() == 0);
  assert(mgr.heal_count() == 0);

  std::vector<byte> frame = build_cyclic_lrd_frame();
  Result<std::uint16_t> res = mgr.exchange_cyclic_frame(frame, EXPECTED_FULL_WKC);

  assert(res.has_value());
  assert(res.value() == EXPECTED_FULL_WKC);
  assert(mgr.state() == RedundancyState::CIRCULAR_NORMAL);

  // Check payload
  std::size_t p_off = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + 10;
  assert(frame[p_off + 0] == static_cast<byte>(0x11));
  assert(frame[p_off + 1] == static_cast<byte>(0x22));
  assert(frame[p_off + 2] == static_cast<byte>(0x33));
  assert(frame[p_off + 3] == static_cast<byte>(0x44));
  assert(frame[p_off + 4] == static_cast<byte>(0x55));
  assert(frame[p_off + 5] == static_cast<byte>(0x66));

  std::cout << "test_circular_normal passed successfully." << std::endl;
}

/**
 * @brief Test cable break detection, failover to bifurcated mode, and payload merging.
 */
void test_cable_break_and_bifurcated_failover() {
  std::cout << "--- Running test_cable_break_and_bifurcated_failover ---" << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();

  std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create("Slave0", 0, SLAVE_0_ADDR);
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create("Slave1", 1, SLAVE_1_ADDR);
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create("Slave2", 2, SLAVE_2_ADDR);

  bus->attach_slave(s0);
  bus->attach_slave(s1);
  bus->attach_slave(s2);

  configure_slave_input_fmmu(*s0, LOGICAL_BASE_ADDR + 0, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s1, LOGICAL_BASE_ADDR + 2, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s2, LOGICAL_BASE_ADDR + 4, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);

  write_slave_inputs(*s0, 0x11, 0x22);
  write_slave_inputs(*s1, 0x33, 0x44);
  write_slave_inputs(*s2, 0x55, 0x66);

  std::shared_ptr<StubRawSocket> sock = std::make_shared<StubRawSocket>(bus, true);
  RedundancyManager mgr(*sock);

  // Simulate cable break between slave 0 and slave 1
  bus->break_cable_after(0);
  assert(bus->is_cable_broken());

  std::vector<byte> frame = build_cyclic_lrd_frame();

  // On exchange, primary only reaches s0 (WKC = 1), triggering immediate failover to secondary
  // Secondary reaches s2 and s1 (WKC = 2). Total WKC = 1 + 2 = 3!
  Result<std::uint16_t> res = mgr.exchange_cyclic_frame(frame, EXPECTED_FULL_WKC);

  assert(res.has_value());
  assert(res.value() == EXPECTED_FULL_WKC);
  assert(mgr.state() == RedundancyState::BIFURCATED_BROKEN);
  assert(mgr.break_count() == 1);

  // Verify all data was correctly merged without packet drop!
  std::size_t p_off = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + 10;
  assert(frame[p_off + 0] == static_cast<byte>(0x11));
  assert(frame[p_off + 1] == static_cast<byte>(0x22));
  assert(frame[p_off + 2] == static_cast<byte>(0x33));
  assert(frame[p_off + 3] == static_cast<byte>(0x44));
  assert(frame[p_off + 4] == static_cast<byte>(0x55));
  assert(frame[p_off + 5] == static_cast<byte>(0x66));

  // Test steady-state bifurcated exchange with updated data
  write_slave_inputs(*s0, 0xAA, 0xBB);
  write_slave_inputs(*s2, 0xCC, 0xDD);

  std::vector<byte> frame2 = build_cyclic_lrd_frame();
  Result<std::uint16_t> res2 = mgr.exchange_cyclic_frame(frame2, EXPECTED_FULL_WKC);

  assert(res2.has_value());
  assert(res2.value() == EXPECTED_FULL_WKC);
  assert(mgr.state() == RedundancyState::BIFURCATED_BROKEN);

  assert(frame2[p_off + 0] == static_cast<byte>(0xAA));
  assert(frame2[p_off + 1] == static_cast<byte>(0xBB));
  assert(frame2[p_off + 2] == static_cast<byte>(0x33));
  assert(frame2[p_off + 3] == static_cast<byte>(0x44));
  assert(frame2[p_off + 4] == static_cast<byte>(0xCC));
  assert(frame2[p_off + 5] == static_cast<byte>(0xDD));

  std::cout << "test_cable_break_and_bifurcated_failover passed successfully." << std::endl;
}

/**
 * @brief Test cable healing detection via secondary probe datagram and transition back to normal.
 */
void test_loop_healing() {
  std::cout << "--- Running test_loop_healing ---" << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();

  std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create("Slave0", 0, SLAVE_0_ADDR);
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create("Slave1", 1, SLAVE_1_ADDR);
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create("Slave2", 2, SLAVE_2_ADDR);

  bus->attach_slave(s0);
  bus->attach_slave(s1);
  bus->attach_slave(s2);

  configure_slave_input_fmmu(*s0, LOGICAL_BASE_ADDR + 0, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s1, LOGICAL_BASE_ADDR + 2, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);
  configure_slave_input_fmmu(*s2, LOGICAL_BASE_ADDR + 4, FMMU_INPUT_LEN, PHYSICAL_INPUT_ADDR);

  std::shared_ptr<StubRawSocket> sock = std::make_shared<StubRawSocket>(bus, true);
  RedundancyManager mgr(*sock);

  // Break cable after slave 1
  bus->break_cable_after(1);

  std::vector<byte> frame = build_cyclic_lrd_frame();
  (void)mgr.exchange_cyclic_frame(frame, EXPECTED_FULL_WKC);
  assert(mgr.state() == RedundancyState::BIFURCATED_BROKEN);

  // Probe while cable is broken: should fail to heal
  bool healed_broken = mgr.check_healing_probe();
  assert(!healed_broken);
  assert(mgr.state() == RedundancyState::BIFURCATED_BROKEN);

  // Restore physical cable!
  bus->restore_cable();
  assert(!bus->is_cable_broken());

  // Probe 1: confirmation 1 (needs DEFAULT_HEAL_CONFIRM_COUNT = 2)
  bool probe1 = mgr.check_healing_probe();
  assert(!probe1);
  assert(mgr.state() == RedundancyState::BIFURCATED_BROKEN);

  // Probe 2: confirmation 2 -> healing verified!
  bool probe2 = mgr.check_healing_probe();
  assert(probe2);
  assert(mgr.state() == RedundancyState::CIRCULAR_NORMAL);
  assert(mgr.heal_count() == 1);

  // Subsequent cyclic frame runs in pure CIRCULAR_NORMAL
  std::vector<byte> frame_healed = build_cyclic_lrd_frame();
  Result<std::uint16_t> res = mgr.exchange_cyclic_frame(frame_healed, EXPECTED_FULL_WKC);
  assert(res.has_value());
  assert(res.value() == EXPECTED_FULL_WKC);
  assert(mgr.state() == RedundancyState::CIRCULAR_NORMAL);

  std::cout << "test_loop_healing passed successfully." << std::endl;
}

int main() {
  test_circular_normal();
  test_cable_break_and_bifurcated_failover();
  test_loop_healing();

  std::cout << "\n============================================\n";
  std::cout << "All Cable Redundancy Loop Tests Passed 100%!";
  std::cout << "\n============================================\n";
  return 0;
}
