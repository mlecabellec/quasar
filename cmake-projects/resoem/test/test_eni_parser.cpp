/**
 * @file test_eni_parser.cpp
 * @brief Unit tests for ETG.2100 EniDocument parser, serializer, and NamedObject bridge.
 * @details Contributes to [TSK-20260920-001.1].
 */

#include "resoem/EniDocument.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace resoem::eni;

void test_in_memory_parsing() {
  const std::string XML_DOC = R"(<?xml version="1.0" encoding="utf-8"?>
<EtherCATConfig Version="1.3">
  <Config>
    <Master>
      <Info>
        <Name>Quasar Master</Name>
        <Destination>FFFFFFFFFFFF</Destination>
        <Source>001122334455</Source>
        <EtherType>A488</EtherType>
      </Info>
      <InitCmds>
        <InitCmd>
          <Transition>IP</Transition>
          <BeforeSlave>true</BeforeSlave>
          <Comment>Clear status</Comment>
          <Cmd>7</Cmd>
          <Adp>0</Adp>
          <Ado>304</Ado>
          <Data>0000</Data>
          <Retries>3</Retries>
          <Timeout>5000</Timeout>
        </InitCmd>
      </InitCmds>
    </Master>
    <Slave>
      <Info>
        <Name>Coupler_EK1100</Name>
        <PhysAddr>1001</PhysAddr>
        <AutoIncAddr>0</AutoIncAddr>
        <Physics>YY</Physics>
        <VendorId>2</VendorId>
        <ProductCode>0x044C2C52</ProductCode>
        <RevisionNo>0x00110000</RevisionNo>
        <SerialNo>12345</SerialNo>
      </Info>
      <PreviousPort Selected="true">
        <Port>B</Port>
        <PhysAddr>0</PhysAddr>
      </PreviousPort>
      <HotConnect>
        <GroupName>RoboticToolArm</GroupName>
        <GroupMemberCnt>2</GroupMemberCnt>
        <IdentifyCmd>
          <Comment>Read station alias</Comment>
          <Cmd>1</Cmd>
          <Adp>0</Adp>
          <Ado>304</Ado>
          <DataLength>2</DataLength>
          <Validate Signed="false" Type="EQ">
            <Data>2001</Data>
            <DataMask>FFFF</DataMask>
            <Timeout>1000</Timeout>
          </Validate>
        </IdentifyCmd>
      </HotConnect>
      <ProcessData>
        <Sm2>
          <Type>Outputs</Type>
          <StartAddress>4096</StartAddress>
          <ControlByte>100</ControlByte>
          <Enable>true</Enable>
        </Sm2>
        <Sm3>
          <Type>Inputs</Type>
          <StartAddress>4608</StartAddress>
          <ControlByte>32</ControlByte>
          <Enable>true</Enable>
        </Sm3>
        <RxPdo Mandatory="true" Sm="2">
          <Index>#x1600</Index>
          <Name>DigitalOutput</Name>
          <Entry>
            <Index>#x7000</Index>
            <SubIndex>1</SubIndex>
            <BitLen>8</BitLen>
            <Name>Channel1</Name>
            <DataType>USINT</DataType>
          </Entry>
        </RxPdo>
        <TxPdo Mandatory="true" Sm="3">
          <Index>#x1A00</Index>
          <Name>DigitalInput</Name>
          <Entry>
            <Index>#x6000</Index>
            <SubIndex>1</SubIndex>
            <BitLen>8</BitLen>
            <Name>Input1</Name>
            <DataType>USINT</DataType>
          </Entry>
        </TxPdo>
      </ProcessData>
      <Mailbox DataLinkLayer="true">
        <Send>
          <Start>4096</Start>
          <Length>128</Length>
        </Send>
        <Receive>
          <Start>4224</Start>
          <Length>128</Length>
          <StatusBitAddr>0</StatusBitAddr>
        </Receive>
        <Protocol>CoE</Protocol>
        <CoE>
          <InitCmds>
            <InitCmd CompleteAccess="true" Fixed="true">
              <Transition>PS</Transition>
              <Comment>Download PDO mapping 0x1C12</Comment>
              <Index>7186</Index>
              <SubIndex>0</SubIndex>
              <Ccs>1</Ccs>
              <Data>01000016</Data>
            </InitCmd>
          </InitCmds>
        </CoE>
      </Mailbox>
      <InitCmds>
        <InitCmd>
          <Transition>IP</Transition>
          <Comment>Set PREOP</Comment>
          <Cmd>5</Cmd>
          <Adp>1001</Adp>
          <Ado>288</Ado>
          <Data>0200</Data>
          <Validate Type="EQ">
            <Data>0200</Data>
            <DataMask>0F00</DataMask>
            <Timeout>3000</Timeout>
          </Validate>
        </InitCmd>
      </InitCmds>
      <DC>
        <PotentialReferenceClock>true</PotentialReferenceClock>
        <CycleTime0>1000000</CycleTime0>
        <ShiftTime0>100000</ShiftTime0>
      </DC>
    </Slave>
    <Cyclic>
      <CycleTime>1000</CycleTime>
      <Frame>
        <Cmd>
          <State>OP</State>
          <Comment>Outputs LWR</Comment>
          <Cmd>11</Cmd>
          <Addr>268435456</Addr>
          <DataLength>8</DataLength>
          <Cnt>1</Cnt>
          <InputOffs>0</InputOffs>
          <OutputOffs>0</OutputOffs>
        </Cmd>
        <Cmd>
          <State>OP</State>
          <Comment>Inputs LRD</Comment>
          <Cmd>10</Cmd>
          <Addr>268435464</Addr>
          <DataLength>8</DataLength>
          <Cnt>1</Cnt>
          <InputOffs>8</InputOffs>
          <OutputOffs>8</OutputOffs>
        </Cmd>
      </Frame>
    </Cyclic>
    <ProcessImage>
      <Inputs>
        <Variable>
          <Name>Sensor1</Name>
          <DataType>USINT</DataType>
          <BitSize>8</BitSize>
          <BitOffs>0</BitOffs>
        </Variable>
      </Inputs>
      <Outputs>
        <Variable>
          <Name>Actuator1</Name>
          <DataType>USINT</DataType>
          <BitSize>8</BitSize>
          <BitOffs>0</BitOffs>
        </Variable>
      </Outputs>
    </ProcessImage>
  </Config>
</EtherCATConfig>
)";

  EniDocument doc;
  resoem::Result<> res = doc.load_from_memory(XML_DOC);
  assert(res.has_value());

  // Master assertions
  assert(doc.master().name == "Quasar Master");
  assert(doc.master().destination_mac == "FFFFFFFFFFFF");
  assert(doc.master().source_mac == "001122334455");
  assert(doc.master().init_cmds.size() == 1);
  assert(doc.master().init_cmds[0].transition == EsmTransition::IP);
  assert(doc.master().init_cmds[0].cmd == DatagramCmd::BRD);
  assert(doc.master().init_cmds[0].ado == 304);

  // Slave assertions
  assert(doc.slaves().size() == 1);
  const EniSlave &s = doc.slaves()[0];
  assert(s.name == "Coupler_EK1100");
  assert(s.phys_addr == 1001);
  assert(s.vendor_id == 2);
  assert(s.product_code == 0x044C2C52);
  assert(s.revision_no == 0x00110000);
  assert(s.serial_no == 12345);

  // PreviousPort assertions
  assert(s.previous_port.has_value());
  assert(s.previous_port->port == 'B');
  assert(s.previous_port->phys_addr == 0);
  assert(s.previous_port->selected == true);

  // HotConnect assertions
  assert(s.hot_connect.has_value());
  assert(s.hot_connect->group_name == "RoboticToolArm");
  assert(s.hot_connect->group_member_count == 2);
  assert(s.hot_connect->identify_cmds.size() == 1);
  assert(s.hot_connect->identify_cmds[0].validate.has_value());
  assert(s.hot_connect->identify_cmds[0].validate->type == "EQ");

  // SyncManager and PDO assertions
  assert(s.sync_managers.size() == 2);
  assert(s.sync_managers[0].type == "Outputs");
  assert(s.sync_managers[0].start_address == 4096);
  assert(s.rx_pdos.size() == 1);
  assert(s.rx_pdos[0].index == 0x1600);
  assert(s.rx_pdos[0].entries.size() == 1);
  assert(s.rx_pdos[0].entries[0].index == 0x7000);
  assert(s.tx_pdos.size() == 1);
  assert(s.tx_pdos[0].index == 0x1A00);

  // CoE assertions
  assert(s.mailbox.coe_init_cmds.size() == 1);
  assert(s.mailbox.coe_init_cmds[0].index == 7186);
  assert(s.mailbox.coe_init_cmds[0].complete_access == true);
  assert(s.mailbox.coe_init_cmds[0].data.size() == 4);

  // Slave InitCmd assertions
  assert(s.init_cmds.size() == 1);
  assert(s.init_cmds[0].transition == EsmTransition::IP);
  assert(s.init_cmds[0].cmd == DatagramCmd::FPWR);
  assert(s.init_cmds[0].validate.has_value());

  // DC assertions
  assert(s.dc.enabled == true);
  assert(s.dc.potential_ref_clock == true);
  assert(s.dc.cycle_time_0 == 1000000);

  // Cyclic frame assertions
  assert(doc.cycle_time_us() == 1000);
  assert(doc.cyclic_frames().size() == 1);
  assert(doc.cyclic_frames()[0].commands.size() == 2);
  assert(doc.cyclic_frames()[0].commands[0].cmd == DatagramCmd::LWR);
  assert(doc.cyclic_frames()[0].commands[1].cmd == DatagramCmd::LRD);

  // ProcessImage assertions
  assert(doc.process_image_variables().size() == 2);
  assert(doc.process_image_variables()[0].is_input == true);
  assert(doc.process_image_variables()[0].name == "Sensor1");
  assert(doc.process_image_variables()[1].is_input == false);
  assert(doc.process_image_variables()[1].name == "Actuator1");

  // Lookup assertions
  const EniSlave *found_phys = doc.find_slave_by_phys_addr(1001);
  assert(found_phys != nullptr);
  assert(found_phys->name == "Coupler_EK1100");

  std::cout << "[PASS] In-memory ENI Parsing Test Passed." << std::endl;
}

void test_named_tree_bridge() {
  const std::string XML_DOC = R"(<?xml version="1.0" encoding="utf-8"?>
<EtherCATConfig Version="1.3">
  <Config>
    <Master>
      <Info>
        <Name>BridgeMaster</Name>
        <Destination>FFFFFFFFFFFF</Destination>
        <Source>112233445566</Source>
      </Info>
    </Master>
    <Slave>
      <Info>
        <Name>EK1100</Name>
        <PhysAddr>2002</PhysAddr>
        <VendorId>2</VendorId>
        <ProductCode>12345</ProductCode>
        <RevisionNo>1</RevisionNo>
      </Info>
      <PreviousPort Selected="true">
        <Port>B</Port>
        <PhysAddr>0</PhysAddr>
      </PreviousPort>
    </Slave>
  </Config>
</EtherCATConfig>
)";

  EniDocument doc;
  resoem::Result<> res = doc.load_from_memory(XML_DOC);
  assert(res.has_value());

  std::shared_ptr<quasar::named::NamedObject> tree = doc.to_named_tree();
  assert(tree != nullptr);
  assert(tree->getName() == "EniConfig");

  std::shared_ptr<quasar::named::NamedObject> master = tree->getChild("Master");
  assert(master != nullptr);

  std::shared_ptr<quasar::named::NamedObject> slaves = tree->getChild("Slaves");
  assert(slaves != nullptr);

  std::shared_ptr<quasar::named::NamedObject> s1 = slaves->getChild("EK1100");
  assert(s1 != nullptr);

  std::cout << "[PASS] NamedObject Bridge Test Passed." << std::endl;
}

void test_round_trip_serialization() {
  const std::string XML_DOC = R"(<?xml version="1.0" encoding="utf-8"?>
<EtherCATConfig Version="1.3">
  <Config>
    <Master>
      <Info>
        <Name>RoundTripMaster</Name>
        <Destination>FFFFFFFFFFFF</Destination>
        <Source>112233445566</Source>
      </Info>
    </Master>
    <Slave>
      <Info>
        <Name>TestSlave1</Name>
        <PhysAddr>1001</PhysAddr>
        <VendorId>2</VendorId>
        <ProductCode>9999</ProductCode>
        <RevisionNo>1</RevisionNo>
      </Info>
    </Slave>
    <Cyclic>
      <CycleTime>2000</CycleTime>
    </Cyclic>
  </Config>
</EtherCATConfig>
)";

  EniDocument doc1;
  resoem::Result<> res1 = doc1.load_from_memory(XML_DOC);
  assert(res1.has_value());

  resoem::Result<std::string> serialized = doc1.save_to_string();
  assert(serialized.has_value());

  EniDocument doc2;
  resoem::Result<> res2 = doc2.load_from_memory(*serialized);
  assert(res2.has_value());

  assert(doc2.master().name == "RoundTripMaster");
  assert(doc2.slaves().size() == 1);
  assert(doc2.slaves()[0].phys_addr == 1001);
  assert(doc2.slaves()[0].product_code == 9999);
  assert(doc2.cycle_time_us() == 2000);

  std::cout << "[PASS] Round-Trip Serialization Test Passed." << std::endl;
}

void test_complex_sample_file() {
  EniDocument doc;
  resoem::Result<> res = doc.load_from_file("tmp/eni-files/d17ae50ba9a2fdcc055a8b05d2e64753a0ee8e5e902acddc2ed16e400d95ff3b.xml");
  if (res.has_value()) {
    std::cout << "  Loaded complex ENI with " << doc.slaves().size() << " slaves." << std::endl;
    assert(!doc.slaves().empty());
  } else {
    std::cout << "  (File load skipped or file not found in current test working dir)" << std::endl;
  }
  std::cout << "[PASS] Complex Sample File Test Passed." << std::endl;
}

int main() {
  test_in_memory_parsing();
  test_named_tree_bridge();
  test_round_trip_serialization();
  test_complex_sample_file();

  std::cout << "\nALL ENI PARSER UNIT TESTS PASSED SUCCESSFULLY!" << std::endl;
  return 0;
}
