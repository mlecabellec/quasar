/**
 * @file test_eni_generator_comparator.cpp
 * @brief Unit tests for EniGenerator and EniComparator engines.
 * @details Contributes to [TSK-20260920-001.3] and [TSK-20260920-001.4].
 */

#include "resoem/EniComparator.hpp"
#include "resoem/EniDocument.hpp"
#include "resoem/EniGenerator.hpp"
#include "resoem/virtual/SimulatedSlave.hpp"
#include "resoem/virtual/StubRawSocket.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

using namespace resoem;
using namespace resoem::eni;
using namespace resoem::virtual_bus;

namespace {

std::shared_ptr<VirtualEtherCATBus> setup_test_bus() {
  std::shared_ptr<VirtualEtherCATBus> bus = VirtualEtherCATBus::create();

  std::shared_ptr<SimulatedSlave> s0 = SimulatedSlave::create(
      "Coupler_EK1100", 0x00000002, 0x044C2C52, 0x00100000, 0x00000001);
  std::shared_ptr<SimulatedSlave> s1 = SimulatedSlave::create(
      "DigOut_EL2004", 0x00000002, 0x07D43052, 0x00100000, 0x00000002);
  std::shared_ptr<SimulatedSlave> s2 = SimulatedSlave::create(
      "DigIn_EL1004", 0x00000002, 0x03EC3052, 0x00100000, 0x00000003);

  // Configure SM2 (Outputs) on s1 at 0x0810
  // [0..1]=0x0F00, [2..3]=0x0001, [4]=0x24 (Outputs), [6]=0x01 (Enable)
  s1->write_register(0x0810, 0x00);
  s1->write_register(0x0811, 0x0F);
  s1->write_register(0x0812, 0x01);
  s1->write_register(0x0813, 0x00);
  s1->write_register(0x0814, 0x24);
  s1->write_register(0x0816, 0x01);

  // Configure SM3 (Inputs) on s2 at 0x0818
  // [0..1]=0x0100, [2..3]=0x0001, [4]=0x20 (Inputs), [6]=0x01 (Enable)
  s2->write_register(0x0818, 0x00);
  s2->write_register(0x0819, 0x01);
  s2->write_register(0x081A, 0x01);
  s2->write_register(0x081B, 0x00);
  s2->write_register(0x081C, 0x20);
  s2->write_register(0x081E, 0x01);

  bus->attach_slave(s0);
  bus->attach_slave(s1);
  bus->attach_slave(s2);

  return bus;
}

} // anonymous namespace

static void test_eni_generation_and_export() {
  std::cout << "--- Running test_eni_generation_and_export ---" << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = setup_test_bus();
  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);

  EniGenerator generator(*socket);
  Result<EniDocument> gen_res = generator.generate("TestScannedMaster");

  assert(gen_res.has_value());
  const EniDocument &doc = gen_res.value();

  // Verify Master
  assert(doc.master().name == "TestScannedMaster");
  assert(!doc.master().init_cmds.empty());

  // Verify Slaves
  assert(doc.slaves().size() == 3);
  assert(doc.slaves()[0].vendor_id == 0x00000002);
  assert(doc.slaves()[0].product_code == 0x044C2C52);
  assert(doc.slaves()[1].product_code == 0x07D43052);
  assert(doc.slaves()[2].product_code == 0x03EC3052);

  // Verify PDO generation
  assert(!doc.slaves()[1].rx_pdos.empty());
  assert(!doc.slaves()[2].tx_pdos.empty());

  // Verify Cyclic Frame
  assert(!doc.cyclic_frames().empty());
  assert(doc.cyclic_frames()[0].commands[0].cmd == DatagramCmd::LRW);
  assert(doc.cyclic_frames()[0].commands[0].expected_wkc == 2);

  // Verify XML Serialization
  std::string xml = generator.serialize_to_xml_string(doc);
  assert(!xml.empty());
  assert(xml.find("<EtherCATConfig>") != std::string::npos);
  assert(xml.find("TestScannedMaster") != std::string::npos);
  assert(xml.find("044C2C52") != std::string::npos || xml.find("72100946") != std::string::npos);

  // Round-trip parse
  EniDocument parsed_rt;
  Result<> rt_res = parsed_rt.load_from_memory(xml);
  assert(rt_res.has_value());
  assert(parsed_rt.slaves().size() == 3);

  std::cout << "test_eni_generation_and_export passed successfully." << std::endl;
}

static void test_eni_comparator_matching_and_mismatches() {
  std::cout << "--- Running test_eni_comparator_matching_and_mismatches ---" << std::endl;

  std::shared_ptr<VirtualEtherCATBus> bus = setup_test_bus();
  std::shared_ptr<StubRawSocket> socket = std::make_shared<StubRawSocket>(bus);

  EniGenerator generator(*socket);
  Result<EniDocument> gen_res = generator.generate("RefMaster");
  assert(gen_res.has_value());

  EniDocument ref_doc = gen_res.value();
  EniDocument act_doc = ref_doc; // identical clone

  // 1. Exact match test
  ComparisonReport rep_exact = EniComparator::compare_documents(ref_doc, act_doc, ComparisonLevel::ALL);
  assert(rep_exact.matched);
  assert(rep_exact.mismatches.empty());

  // 2. Direct Live Bus Comparison
  ComparisonReport rep_live = EniComparator::compare_live_bus(ref_doc, *socket, ComparisonLevel::ALL);
  assert(rep_live.matched);
  assert(rep_live.mismatches.empty());

  // 3. Test TYPE Mismatch (Product Code mutated)
  act_doc.slaves()[1].product_code = 0x99999999;
  ComparisonReport rep_type = EniComparator::compare_documents(ref_doc, act_doc, ComparisonLevel::TYPE);
  assert(!rep_type.matched);
  assert(rep_type.mismatches.size() == 1);
  assert(rep_type.mismatches[0].item_name == "ProductCode");

  // 4. Test TOPOLOGY Mismatch (PhysAddr mutated)
  act_doc = ref_doc;
  act_doc.slaves()[2].phys_addr = 0x9999;
  ComparisonReport rep_topo = EniComparator::compare_documents(ref_doc, act_doc, ComparisonLevel::TOPOLOGY);
  assert(!rep_topo.matched);
  assert(rep_topo.mismatches.size() == 1);
  assert(rep_topo.mismatches[0].item_name == "PhysAddr");

  // 5. Test Slave Count Mismatch
  act_doc = ref_doc;
  act_doc.slaves().pop_back(); // only 2 slaves instead of 3
  ComparisonReport rep_count = EniComparator::compare_documents(ref_doc, act_doc, ComparisonLevel::TOPOLOGY);
  assert(!rep_count.matched);
  assert(rep_count.mismatches.size() == 1);
  assert(rep_count.mismatches[0].item_name == "SlaveCount");

  // 6. Test HOT_CONNECT Mismatch
  act_doc = ref_doc;
  EniHotConnect hc;
  hc.group_name = "MismatchGroup";
  act_doc.slaves()[0].hot_connect = hc;
  ComparisonReport rep_hc = EniComparator::compare_documents(ref_doc, act_doc, ComparisonLevel::HOT_CONNECT);
  assert(!rep_hc.matched);
  assert(rep_hc.mismatches.size() == 1);
  assert(rep_hc.mismatches[0].item_name == "HotConnectStatus");

  // Verify format_summary output
  std::string summary = rep_type.format_summary();
  assert(!summary.empty());
  assert(summary.find("MISMATCH DETECTED") != std::string::npos);

  std::cout << "test_eni_comparator_matching_and_mismatches passed successfully." << std::endl;
}

int main() {
  std::cout << "=== RUNNING ENI GENERATOR & COMPARATOR TESTS ===" << std::endl;

  test_eni_generation_and_export();
  test_eni_comparator_matching_and_mismatches();

  std::cout << "\nALL ENI GENERATOR & COMPARATOR TESTS PASSED 100%!" << std::endl;
  return 0;
}
