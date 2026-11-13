/**
 * @file test_compliance_and_stress.cpp
 * @brief Stress, concurrency, and C++23 compliance verification tests for resoem.
 * @feature [TSK-20261113-002]
 */

#include "resoem/CoEHandler.hpp"
#include "resoem/EniConfigurator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/EtherCATTypes.hpp"
#include "resoem/FoEHandler.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/ProcessImage.hpp"
#include "resoem/SoEHandler.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace resoem_stress_test {

constexpr std::uint32_t VENDOR_BECKHOFF = 0x00000002U;
constexpr std::uint32_t PRODUCT_COUPLER = 0x044C2C52U;
constexpr std::uint32_t PRODUCT_DO = 0x07D43052U;
constexpr std::uint32_t PRODUCT_DI = 0x03EC3052U;
constexpr std::uint32_t FOE_HEADER_SIZE_BYTES = 6U;
constexpr std::uint32_t TEST_PASSWORD = 0x12345678U;
constexpr std::uint32_t TEST_PACKET_NO = 0x0000002AU;
constexpr std::uint32_t TEST_ERROR_CODE = 0x00008005U;
constexpr std::uint8_t BYTE_MASK = 0xFFU;
constexpr std::uint32_t SHIFT_8 = 8U;
constexpr std::uint32_t SHIFT_16 = 16U;
constexpr std::uint32_t SHIFT_24 = 24U;
constexpr std::uint32_t REVISION_DEFAULT = 0x00100000U;
constexpr std::uint16_t PHYS_ADDR_SLAVE0 = 0x1001U;
constexpr std::uint16_t PHYS_ADDR_SLAVE1 = 0x1002U;
constexpr std::uint16_t PHYS_ADDR_SLAVE2 = 0x1003U;
constexpr std::uint16_t SM_OUT_START_ADDR = 0x0F00U;
constexpr std::uint8_t SM_OUT_CONTROL = 0x24U;
constexpr std::uint8_t SM_IN_CONTROL = 0x20U;
constexpr std::uint16_t SM_IN_START_ADDR = 0x0E00U;
constexpr std::size_t EXPECTED_SLAVES = 3U;
constexpr std::uint32_t CYCLIC_ITERATIONS = 500U;
constexpr std::uint32_t FAULT_CYCLES = 20U;
constexpr std::size_t FAULT_SPLIT_INDEX = 1U;
constexpr std::size_t FAULT_REMAINING_SLAVES = 2U;
constexpr std::size_t WORKER_COUNT = 2U;
constexpr std::size_t HANDLER_CALLS_PER_THREAD = 10U;
constexpr std::size_t BUFFER_SIZE = 32U;
constexpr std::uint16_t MBX_IN_ADDR = 0x1800U;
constexpr std::uint16_t MBX_OUT_ADDR = 0x1C00U;
constexpr std::uint16_t MBX_LEN = 128U;
constexpr std::uint16_t TEST_SDO_INDEX = 0x1018U;
constexpr std::uint8_t TEST_SDO_SUBINDEX = 0x01U;
constexpr std::uint16_t TEST_SOE_IDN = 0x001EU;
constexpr std::size_t PROCESS_IMAGE_BYTES = 2U;
constexpr std::int64_t INITIAL_SENSOR_VALUE = 0x5AU;
constexpr std::uint8_t OUTPUT_PATTERN_MASK = 0x0FU;
constexpr int TIMEOUT_MS = 2;

/// @brief Always-active runtime check invariant (safe under NDEBUG).
static void require_true(bool condition, const char *message) {
  if (condition == false) {
    throw std::runtime_error{message != nullptr ? message : "Test invariant failed"};
  }
}

/// @brief Verify union-free FoE wire header binary layout and accessors.
static void test_union_free_foe_header() {
  std::cout << "[TEST] Running test_union_free_foe_header..." << std::endl;

  // Validate packed binary size matches ETG.1000 FoE header specification.
  require_true(sizeof(resoem::foe::Header) == FOE_HEADER_SIZE_BYTES,
               "foe::Header size mismatch");

  resoem::foe::Header hdr{};
  hdr.opcode = resoem::foe::Opcode::WRQ;
  hdr.reserved = 0U;
  hdr.set_password(TEST_PASSWORD);
  require_true(hdr.password() == TEST_PASSWORD, "FoE password roundtrip failed");

  // Verify DATA/ACK sequence accessor shares param field without UB.
  hdr.opcode = resoem::foe::Opcode::DATA;
  hdr.set_packet_no(TEST_PACKET_NO);
  require_true(hdr.packet_no() == TEST_PACKET_NO, "FoE packet_no roundtrip failed");

  // Verify ERR frame error code roundtrip and serialized bytes.
  hdr.opcode = resoem::foe::Opcode::ERR;
  hdr.set_error_code(TEST_ERROR_CODE);
  require_true(hdr.error_code() == TEST_ERROR_CODE, "FoE error_code roundtrip failed");

  // Inspect packed representation via memcpy to avoid aliasing UB.
  std::array<std::uint8_t, FOE_HEADER_SIZE_BYTES> wire{};
  std::memcpy(wire.data(), &hdr, wire.size());
  require_true(wire[0] == static_cast<std::uint8_t>(resoem::foe::Opcode::ERR),
               "FoE wire opcode byte mismatch");
  require_true(wire[2] == static_cast<std::uint8_t>(TEST_ERROR_CODE & BYTE_MASK),
               "FoE wire LSB mismatch");
  require_true(
      wire[3] == static_cast<std::uint8_t>((TEST_ERROR_CODE >> SHIFT_8) & BYTE_MASK),
      "FoE wire byte 3 mismatch");
  require_true(
      wire[4] == static_cast<std::uint8_t>((TEST_ERROR_CODE >> SHIFT_16) & BYTE_MASK),
      "FoE wire byte 4 mismatch");
  require_true(
      wire[5] == static_cast<std::uint8_t>((TEST_ERROR_CODE >> SHIFT_24) & BYTE_MASK),
      "FoE wire MSB mismatch");

  std::cout << "  [PASS] foe::Header union-free layout and accessors verified."
            << std::endl;
}

/// @brief Verify high-iteration cyclic exchange and fault/heal cycles on virtual bus.
static void test_virtual_bus_cyclic_stress() {
  std::cout << "[TEST] Running test_virtual_bus_cyclic_stress..." << std::endl;

  // Build deterministic 3-slave virtual topology.
  std::shared_ptr<resoem::virtual_bus::VirtualEtherCATBus> bus =
      resoem::virtual_bus::VirtualEtherCATBus::create();
  require_true(bus != nullptr, "Failed to create VirtualEtherCATBus");

  std::shared_ptr<resoem::virtual_bus::SimulatedSlave> s0 =
      resoem::virtual_bus::SimulatedSlave::create("StressCoupler", VENDOR_BECKHOFF,
                                                  PRODUCT_COUPLER);
  std::shared_ptr<resoem::virtual_bus::SimulatedSlave> s1 =
      resoem::virtual_bus::SimulatedSlave::create("StressOut", VENDOR_BECKHOFF,
                                                  PRODUCT_DO);
  std::shared_ptr<resoem::virtual_bus::SimulatedSlave> s2 =
      resoem::virtual_bus::SimulatedSlave::create("StressIn", VENDOR_BECKHOFF,
                                                  PRODUCT_DI);

  // Register synthetic Tx/Rx process variables aligned with SM physical RAM addresses.
  s1->add_output_variable("OutByte",
                          static_cast<std::size_t>(SM_OUT_START_ADDR) * SHIFT_8,
                          SHIFT_8, 0);
  s2->add_input_variable("InByte",
                         static_cast<std::size_t>(SM_IN_START_ADDR) * SHIFT_8,
                         SHIFT_8, INITIAL_SENSOR_VALUE);

  bus->attach_slave(s0);
  bus->attach_slave(s1);
  bus->attach_slave(s2);

  // Build matching ENI document so SM/FMMU logical mapping is configured on virtual slaves.
  resoem::eni::EniDocument doc{};

  resoem::eni::EniSlave eni0{};
  eni0.name = "StressCoupler";
  eni0.vendor_id = VENDOR_BECKHOFF;
  eni0.product_code = PRODUCT_COUPLER;
  eni0.revision_no = REVISION_DEFAULT;
  eni0.phys_addr = PHYS_ADDR_SLAVE0;
  doc.slaves().push_back(eni0);

  resoem::eni::EniSlave eni1{};
  eni1.name = "StressOut";
  eni1.vendor_id = VENDOR_BECKHOFF;
  eni1.product_code = PRODUCT_DO;
  eni1.revision_no = REVISION_DEFAULT;
  eni1.phys_addr = PHYS_ADDR_SLAVE1;

  resoem::eni::EniSyncManager sm_out{};
  sm_out.start_address = SM_OUT_START_ADDR;
  sm_out.length = 1U;
  sm_out.control_byte = SM_OUT_CONTROL;
  sm_out.type = "Outputs";
  sm_out.enable = true;
  eni1.sync_managers.push_back(sm_out);

  resoem::eni::EniPdo rx_pdo{};
  rx_pdo.is_tx = false;
  resoem::eni::EniPdoEntry rx_entry{};
  rx_entry.name = "OutByte";
  rx_entry.bit_len = static_cast<std::uint16_t>(SHIFT_8);
  rx_pdo.entries.push_back(rx_entry);
  eni1.rx_pdos.push_back(rx_pdo);
  doc.slaves().push_back(eni1);

  resoem::eni::EniSlave eni2{};
  eni2.name = "StressIn";
  eni2.vendor_id = VENDOR_BECKHOFF;
  eni2.product_code = PRODUCT_DI;
  eni2.revision_no = REVISION_DEFAULT;
  eni2.phys_addr = PHYS_ADDR_SLAVE2;

  resoem::eni::EniSyncManager sm_in{};
  sm_in.start_address = SM_IN_START_ADDR;
  sm_in.length = 1U;
  sm_in.control_byte = SM_IN_CONTROL;
  sm_in.type = "Inputs";
  sm_in.enable = true;
  eni2.sync_managers.push_back(sm_in);

  resoem::eni::EniPdo tx_pdo{};
  tx_pdo.is_tx = true;
  resoem::eni::EniPdoEntry tx_entry{};
  tx_entry.name = "InByte";
  tx_entry.bit_len = static_cast<std::uint16_t>(SHIFT_8);
  tx_pdo.entries.push_back(tx_entry);
  eni2.tx_pdos.push_back(tx_pdo);
  doc.slaves().push_back(eni2);

  resoem::eni::EniCyclicFrame cyc_frame{};
  resoem::eni::EniCyclicCmd cyc_cmd{};
  cyc_cmd.cmd = resoem::eni::DatagramCmd::LRW;
  cyc_cmd.addr = 0U;
  cyc_cmd.data_length = static_cast<std::uint16_t>(PROCESS_IMAGE_BYTES);
  cyc_frame.commands.push_back(cyc_cmd);
  doc.cyclic_frames().push_back(cyc_frame);

  // Initialize virtual socket and configure bus to OP via EniConfigurator.
  std::shared_ptr<resoem::virtual_bus::StubRawSocket> socket =
      std::make_shared<resoem::virtual_bus::StubRawSocket>(bus);
  require_true(socket != nullptr, "Failed to create StubRawSocket");

  resoem::eni::EniConfigurator configurator{*socket, doc};
  resoem::Result<void> cfg_res = configurator.configure_all();
  require_true(cfg_res.has_value() == true, "EniConfigurator::configure_all failed");

  // Prepare 2-byte cyclic process image (byte 0 = DO, byte 1 = DI).
  std::vector<resoem::byte> image{};
  image.resize(PROCESS_IMAGE_BYTES);

  // Execute bounded cyclic exchange loop verifying IO data integrity.
  for (std::uint32_t iter = 0U; iter < CYCLIC_ITERATIONS; ++iter) {
    const std::uint8_t cmd = static_cast<std::uint8_t>(iter & OUTPUT_PATTERN_MASK);
    const std::uint8_t expected_in =
        static_cast<std::uint8_t>((INITIAL_SENSOR_VALUE + iter) & BYTE_MASK);
    s2->set_input_value("InByte", static_cast<std::int64_t>(expected_in));
    image[0] = cmd;
    image[1] = 0U;

    resoem::Result<std::uint16_t> wkc =
        configurator.exchange_cyclic_frame(0U, std::span<resoem::byte>{image});
    require_true(wkc.has_value() == true, "Cyclic process data exchange failed");
    require_true(wkc.value() > 0U, "Cyclic WKC unexpectedly zero");
    require_true(s1->get_output_value("OutByte") == static_cast<std::int64_t>(cmd),
                 "Slave output mismatch during cyclic exchange");
    require_true(image[1] == expected_in,
                 "Master input mismatch during cyclic exchange");
  }

  // Exercise repeated link break and heal detection cycles via Enumerator.
  resoem::Enumerator enumerator{*socket};
  for (std::uint32_t cycle = 0U; cycle < FAULT_CYCLES; ++cycle) {
    bus->break_cable_after(FAULT_SPLIT_INDEX);
    require_true(bus->is_cable_broken() == true, "Cable break flag not active");
    resoem::Result<std::size_t> broken_count = enumerator.enumerate();
    require_true(broken_count.has_value() == true, "Enumeration failed during cable break");
    require_true(broken_count.value() == FAULT_REMAINING_SLAVES,
                 "Broken bus did not isolate downstream slave");

    bus->restore_cable();
    require_true(bus->is_cable_broken() == false, "Cable break flag still set");
    resoem::Result<std::size_t> healed_count = enumerator.enumerate();
    require_true(healed_count.has_value() == true, "Enumeration failed after heal");
    require_true(healed_count.value() == EXPECTED_SLAVES,
                 "Healed bus did not recover full slave count");
  }

  std::cout << "  [PASS] 500 cyclic frames + 20 break/heal topology cycles verified."
            << std::endl;
}

/// @brief Verify timed-mutex thread safety and clean timeout handling across mailbox handlers.
static void test_handler_concurrency_and_timeout_stress() {
  std::cout << "[TEST] Running test_handler_concurrency_and_timeout_stress..."
            << std::endl;

  // Prepare single-slave virtual bus (no mailbox responder configured).
  std::shared_ptr<resoem::virtual_bus::VirtualEtherCATBus> bus =
      resoem::virtual_bus::VirtualEtherCATBus::create();
  require_true(bus != nullptr, "Failed to create bus for handler stress");
  std::shared_ptr<resoem::virtual_bus::SimulatedSlave> slave_node =
      resoem::virtual_bus::SimulatedSlave::create("MailboxTarget", VENDOR_BECKHOFF,
                                                  PRODUCT_COUPLER);
  bus->attach_slave(slave_node);

  std::shared_ptr<resoem::virtual_bus::StubRawSocket> socket =
      std::make_shared<resoem::virtual_bus::StubRawSocket>(bus);
  resoem::Enumerator enumerator{*socket};
  resoem::Result<std::size_t> enum_res = enumerator.enumerate();
  require_true(enum_res.has_value() == true, "Handler stress enumeration failed");
  require_true(enumerator.slaves().empty() == false, "No slave discovered");

  resoem::MailboxHandler mailbox{*socket};
  resoem::CoEHandler coe{mailbox};
  resoem::FoEHandler foe{mailbox};
  resoem::SoEHandler soe{mailbox};

  const resoem::SlaveInfo template_slave = enumerator.slaves()[0];
  std::atomic<std::size_t> completed_calls{0U};
  std::atomic<bool> unexpected_exception{false};

  // Launch parallel workers exercising guarded methods concurrently.
  std::vector<std::thread> workers{};
  const std::chrono::microseconds short_timeout =
      std::chrono::milliseconds(TIMEOUT_MS);

  for (std::size_t t = 0U; t < WORKER_COUNT; ++t) {
    workers.emplace_back([&coe, &foe, &soe, template_slave, short_timeout,
                          &completed_calls, &unexpected_exception, t]() {
      try {
        resoem::SlaveInfo local_slave = template_slave;
        local_slave.mbx_in_offset = MBX_IN_ADDR;
        local_slave.mbx_in_length = MBX_LEN;
        local_slave.mbx_out_offset = MBX_OUT_ADDR;
        local_slave.mbx_out_length = MBX_LEN;

        std::vector<resoem::byte> buf{};
        buf.resize(BUFFER_SIZE);

        // Interleave CoE, FoE, SoE operations under contention.
        for (std::size_t i = 0U; i < HANDLER_CALLS_PER_THREAD; ++i) {
          coe.set_verbose(static_cast<int>(t & 1U));
          resoem::Result<std::size_t> coe_res =
              coe.sdo_read(local_slave, TEST_SDO_INDEX, TEST_SDO_SUBINDEX,
                           std::span<resoem::byte>{buf}, false, short_timeout);
          (void)coe_res;

          resoem::Result<std::vector<resoem::byte>> foe_res =
              foe.read_file(local_slave, "test.bin", 0U, short_timeout);
          (void)foe_res;

          resoem::Result<std::size_t> soe_res =
              soe.read(local_slave, 0U, 1U, TEST_SOE_IDN,
                       std::span<resoem::byte>{buf}, short_timeout);
          (void)soe_res;

          completed_calls.fetch_add(1U);
        }
      } catch (...) {
        unexpected_exception.store(true);
      }
    });
  }

  // Join all worker threads cleanly without unbounded wait assumptions.
  for (std::size_t t = 0U; t < workers.size(); ++t) {
    if (workers[t].joinable() == true) {
      workers[t].join();
    }
  }

  require_true(unexpected_exception.load() == false,
               "Concurrent handler invocation threw an unexpected exception");
  require_true(completed_calls.load() == WORKER_COUNT * HANDLER_CALLS_PER_THREAD,
               "Not all concurrent handler calls finished");

  std::cout << "  [PASS] Multi-threaded CoE/FoE/SoE timed-mutex stress completed ("
            << completed_calls.load() << " batches)." << std::endl;
}

} // namespace resoem_stress_test

/// @brief Entry point executing compliance and stress test suite.
int main() {
  try {
    resoem_stress_test::test_union_free_foe_header();
    resoem_stress_test::test_virtual_bus_cyclic_stress();
    resoem_stress_test::test_handler_concurrency_and_timeout_stress();
  } catch (const std::exception &ex) {
    std::cerr << "[FAIL] " << ex.what() << std::endl;
    return 1;
  }
  std::cout << "[ALL PASSED] resoem compliance & stress suite succeeded."
            << std::endl;
  return 0;
}
