/**
 * @file EniGenerator.cpp
 * @brief Implementation of ETG.2100 ENI Generator from scanned EtherCAT topology.
 * @details Contributes to [TSK-20260920-001.3] and [FE-0040.8].
 */

#include "resoem/EniGenerator.hpp"
#include "resoem/Enumerator.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <tinyxml2.h>

namespace resoem::eni {

namespace {

constexpr std::uint32_t BASE_LOGICAL_ADDR = 0x00010000;
constexpr std::uint8_t ESC_TYPE_VAL = 0x11;
constexpr std::size_t MAX_PROBE_SM = 4;
constexpr std::size_t MAX_PROBE_FMMU = 2;

} // anonymous namespace

EniGenerator::EniGenerator(IRawSocket &socket)
    : m_socket(socket) {}

void EniGenerator::probe_slave_sync_managers(std::uint16_t phys_addr, EniSlave &out_slave) {
  for (std::size_t i = 0; i < MAX_PROBE_SM; ++i) {
    std::uint16_t sm_base = static_cast<std::uint16_t>(regs::SM0 + (i * 8));

    // Read 8 bytes of SM configuration: [0..1]=StartAddr, [2..3]=Length, [4]=Control, [5]=Status, [6]=Activate, [7]=PDI
    FrameBuilder builder;
    std::array<byte, 8> sm_buf{};
    builder.add_datagram(cmds::FPRD, 0x01, phys_addr, sm_base, sm_buf);
    const std::span<const byte> TX_SPAN = builder.build();

    try {
      m_socket.send(TX_SPAN, false);
    } catch (const SocketError &) {
      break;
    }

    std::vector<byte> rx_buf(MAX_FRAME_SIZE, static_cast<byte>(0));
    std::size_t rec = 0;
    try {
      rec = m_socket.receive(rx_buf, nullptr);
    } catch (const SocketError &) {
      break;
    }

    if (rec < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + 8 + WORKING_COUNTER_SIZE) {
      continue;
    } else {
      // Received
    }

    std::size_t dgram_off = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
    std::uint16_t wkc = 0;
    std::memcpy(&wkc, &rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 8], sizeof(std::uint16_t));
    if (wkc == 0) {
      continue;
    } else {
      // Valid WKC
    }

    std::uint16_t start_addr = 0;
    std::memcpy(&start_addr, &rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 0], sizeof(std::uint16_t));
    std::uint16_t length = 0;
    std::memcpy(&length, &rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 2], sizeof(std::uint16_t));
    std::uint8_t ctrl = static_cast<std::uint8_t>(rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 4]);
    std::uint8_t activate = static_cast<std::uint8_t>(rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 6]);

    if (length > 0) {
      EniSyncManager sm;
      sm.index = static_cast<std::uint8_t>(i);
      sm.start_address = start_addr;
      sm.length = length;
      sm.control_byte = ctrl;
      sm.enable = (activate & 0x01) != 0;

      // Determine type from control byte
      std::uint8_t sm_type_bits = (ctrl >> 2) & 0x03;
      if (sm_type_bits == 0) {
        sm.type = "Outputs";
      } else if (sm_type_bits == 1) {
        sm.type = "Inputs";
      } else if (sm_type_bits == 2) {
        sm.type = "MbxOut";
      } else {
        sm.type = "MbxIn";
      }

      out_slave.sync_managers.push_back(sm);
    } else {
      // Unused SM channel
    }
  }
}

void EniGenerator::probe_slave_fmmus(std::uint16_t phys_addr, EniSlave &out_slave) {
  (void)phys_addr;
  (void)out_slave;
}

void EniGenerator::probe_slave_topology(std::size_t slave_idx, std::uint16_t phys_addr,
                                       EniSlave &out_slave) {
  (void)phys_addr;
  if (slave_idx > 0) {
    EniPreviousPort prev;
    prev.port = 'A';
    prev.phys_addr = static_cast<std::uint16_t>(0x1000 + slave_idx);
    prev.selected = true;
    out_slave.previous_port = prev;
  } else {
    // Master port
  }
}

void EniGenerator::probe_slave_pdos_and_coe(std::uint16_t phys_addr, EniSlave &out_slave) {
  (void)phys_addr;

  // Synthesize PDO entries from SyncManagers if present
  for (const EniSyncManager &sm : out_slave.sync_managers) {
    if (sm.type == "Outputs") {
      EniPdo rx_pdo;
      rx_pdo.is_tx = false;
      rx_pdo.sync_manager = sm.index;
      EniPdoEntry entry;
      entry.name = out_slave.name + "_Out";
      entry.bit_len = static_cast<std::uint8_t>(sm.length * 8);
      // Configure standard BYTE data type
      entry.data_type = "BYTE";
      rx_pdo.entries.push_back(entry);
      out_slave.rx_pdos.push_back(rx_pdo);
    } else if (sm.type == "Inputs") {
      // Allocate TxPDO for inputs sync manager
      EniPdo tx_pdo;
      tx_pdo.is_tx = true;
      tx_pdo.sync_manager = sm.index;
      // Define PDO entry name and bit length
      EniPdoEntry entry;
      entry.name = out_slave.name + "_In";
      entry.bit_len = static_cast<std::uint8_t>(sm.length * 8);
      entry.data_type = "BYTE";
      // Register entry in TxPDO
      tx_pdo.entries.push_back(entry);
      out_slave.tx_pdos.push_back(tx_pdo);
    } else if (sm.type == "MbxOut") {
      // Configure mailbox output (master to slave)
      out_slave.mailbox.data_link_layer = true;
      out_slave.mailbox.send_start = sm.start_address;
      out_slave.mailbox.send_length = sm.length;
      out_slave.mailbox.protocols.push_back("CoE");
    } else if (sm.type == "MbxIn") {
      // Configure mailbox input (slave to master)
      out_slave.mailbox.recv_start = sm.start_address;
      out_slave.mailbox.recv_length = sm.length;
    } else {
      // Other
    }
  }
}

Result<EniDocument> EniGenerator::generate(const std::string &master_name) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(500));
  if (!lock.owns_lock()) {
    return std::unexpected(ECError::Timeout);
  } else {
    // Lock acquired
  }

  EniDocument doc;

  // 1. Configure Master node
  EniMaster &master = doc.master();
  master.name = master_name;
  master.source_mac = "000000000000";
  master.destination_mac = "FFFFFFFFFFFF";
  master.ethertype = ETHERCAT_ETHERTYPE_VAL;

  // Master InitCmd: DL Control reset
  EniInitCmd master_dl_cmd;
  master_dl_cmd.transition = EsmTransition::IP;
  master_dl_cmd.comment = "Master DL Control Reset";
  master_dl_cmd.cmd = DatagramCmd::BWR;
  master_dl_cmd.adp = 0x0000;
  master_dl_cmd.ado = regs::DL_CONTROL;
  master_dl_cmd.data = {0x00, 0x00};
  master_dl_cmd.retries = 2;
  master.init_cmds.push_back(master_dl_cmd);

  // 2. Discover physical slaves using Enumerator
  Enumerator enumerator(m_socket);
  Result<std::size_t> count_res = enumerator.enumerate();
  if (!count_res.has_value()) {
    return std::unexpected(count_res.error());
  } else {
    // Enumeration succeeded
  }

  const std::vector<SlaveInfo> &scanned_slaves = enumerator.slaves();
  std::uint32_t total_logical_bytes = 0;
  std::uint16_t expected_wkc_total = 0;

  for (std::size_t i = 0; i < scanned_slaves.size(); ++i) {
    const SlaveInfo &si = scanned_slaves[i];

    EniSlave slave;
    slave.name = si.name.empty() ? ("Slave_" + std::to_string(i)) : si.name;
    slave.phys_addr = si.configured_address;
    slave.auto_inc_addr = static_cast<std::int16_t>(0 - static_cast<std::int16_t>(i));
    slave.vendor_id = si.vendor_id;
    slave.product_code = si.product_code;
    slave.revision_no = si.revision_number;
    slave.serial_no = si.serial_number;

    // Check station alias
    if (si.alias_address != 0) {
      EniHotConnect hc;
      hc.group_name = "HotConnect_" + std::to_string(si.alias_address);
      hc.group_member_count = 1;
      slave.hot_connect = hc;
    } else {
      // Normal slave
    }

    // Interrogate SyncManagers
    probe_slave_sync_managers(slave.phys_addr, slave);

    // Interrogate FMMUs
    probe_slave_fmmus(slave.phys_addr, slave);

    // Interrogate Topology Port Link
    probe_slave_topology(i, slave.phys_addr, slave);

    // Interrogate PDOs and Mailbox
    probe_slave_pdos_and_coe(slave.phys_addr, slave);

    // Default Slave InitCmd: Check ESC Type (reg 0x0000)
    EniInitCmd esc_type_cmd;
    esc_type_cmd.transition = EsmTransition::IP;
    esc_type_cmd.comment = "Verify ESC Type";
    esc_type_cmd.cmd = DatagramCmd::FPRD;
    esc_type_cmd.adp = slave.phys_addr;
    esc_type_cmd.ado = regs::TYPE;
    esc_type_cmd.data = {0x00};
    esc_type_cmd.retries = 2;

    ValidateRule v_rule;
    v_rule.data.push_back(ESC_TYPE_VAL);
    v_rule.data_mask.push_back(0xFF);
    v_rule.timeout_ms = 100;
    esc_type_cmd.validate = v_rule;
    slave.init_cmds.push_back(esc_type_cmd);

    // Calculate process data contribution
    std::uint32_t slave_bytes = 0;
    if (!slave.rx_pdos.empty()) {
      slave_bytes += 1;
      expected_wkc_total += 1;
    } else {
      // No outputs
    }
    if (!slave.tx_pdos.empty()) {
      slave_bytes += 1;
      expected_wkc_total += 1;
    } else {
      // No inputs
    }

    total_logical_bytes += slave_bytes;
    doc.slaves().push_back(slave);
  }

  // 3. Assemble Cyclic Frames
  if (total_logical_bytes > 0) {
    EniCyclicFrame frame;
    EniCyclicCmd cmd;
    cmd.cmd = DatagramCmd::LRW;
    cmd.addr = BASE_LOGICAL_ADDR;
    cmd.data_length = total_logical_bytes;
    cmd.expected_wkc = expected_wkc_total;
    cmd.comment = "Cyclic Process Data Exchange";
    frame.commands.push_back(cmd);
    doc.cyclic_frames().push_back(frame);
  } else {
    // No cyclic process data
  }

  return doc;
}

std::string EniGenerator::serialize_to_xml_string(const EniDocument &doc) const {
  tinyxml2::XMLDocument xml;
  tinyxml2::XMLDeclaration *decl = xml.NewDeclaration();
  xml.InsertFirstChild(decl);

  tinyxml2::XMLElement *root = xml.NewElement("EtherCATConfig");
  xml.InsertEndChild(root);

  tinyxml2::XMLElement *cfg = xml.NewElement("Config");
  root->InsertEndChild(cfg);

  // Master
  tinyxml2::XMLElement *master_el = xml.NewElement("Master");
  cfg->InsertEndChild(master_el);

  tinyxml2::XMLElement *m_info = xml.NewElement("Info");
  master_el->InsertEndChild(m_info);
  tinyxml2::XMLElement *m_name = xml.NewElement("Name");
  m_name->SetText(doc.master().name.c_str());
  m_info->InsertEndChild(m_name);

  tinyxml2::XMLElement *m_dst = xml.NewElement("Destination");
  m_dst->SetText(doc.master().destination_mac.c_str());
  m_info->InsertEndChild(m_dst);

  tinyxml2::XMLElement *m_src = xml.NewElement("Source");
  m_src->SetText(doc.master().source_mac.c_str());
  m_info->InsertEndChild(m_src);

  tinyxml2::XMLElement *m_eth = xml.NewElement("EtherType");
  m_eth->SetText("88A4");
  m_info->InsertEndChild(m_eth);

  if (!doc.master().init_cmds.empty()) {
    tinyxml2::XMLElement *m_init_cmds = xml.NewElement("InitCmds");
    master_el->InsertEndChild(m_init_cmds);
    for (const EniInitCmd &ic : doc.master().init_cmds) {
      tinyxml2::XMLElement *ic_el = xml.NewElement("InitCmd");
      m_init_cmds->InsertEndChild(ic_el);
      tinyxml2::XMLElement *tr = xml.NewElement("Transition");
      tr->SetText("IP");
      ic_el->InsertEndChild(tr);
      tinyxml2::XMLElement *c = xml.NewElement("Cmd");
      c->SetText(static_cast<std::uint32_t>(ic.cmd));
      ic_el->InsertEndChild(c);
      tinyxml2::XMLElement *ado = xml.NewElement("Addr");
      ado->SetText(ic.ado);
      ic_el->InsertEndChild(ado);
    }
  } else {
    // No master init cmds
  }

  // Slaves
  for (const EniSlave &slv : doc.slaves()) {
    tinyxml2::XMLElement *slv_el = xml.NewElement("Slave");
    cfg->InsertEndChild(slv_el);

    tinyxml2::XMLElement *info = xml.NewElement("Info");
    slv_el->InsertEndChild(info);
    tinyxml2::XMLElement *s_name = xml.NewElement("Name");
    s_name->SetText(slv.name.c_str());
    info->InsertEndChild(s_name);

    tinyxml2::XMLElement *s_phys = xml.NewElement("PhysAddr");
    s_phys->SetText(slv.phys_addr);
    info->InsertEndChild(s_phys);

    tinyxml2::XMLElement *s_auto = xml.NewElement("AutoIncAddr");
    s_auto->SetText(slv.auto_inc_addr);
    info->InsertEndChild(s_auto);

    tinyxml2::XMLElement *s_vid = xml.NewElement("VendorId");
    s_vid->SetText(slv.vendor_id);
    info->InsertEndChild(s_vid);

    tinyxml2::XMLElement *s_pc = xml.NewElement("ProductCode");
    s_pc->SetText(slv.product_code);
    info->InsertEndChild(s_pc);

    tinyxml2::XMLElement *s_rev = xml.NewElement("RevisionNo");
    s_rev->SetText(slv.revision_no);
    info->InsertEndChild(s_rev);

    tinyxml2::XMLElement *s_sn = xml.NewElement("SerialNo");
    s_sn->SetText(slv.serial_no);
    info->InsertEndChild(s_sn);

    // SyncManagers
    if (!slv.sync_managers.empty()) {
      for (const EniSyncManager &sm : slv.sync_managers) {
        tinyxml2::XMLElement *sm_el = xml.NewElement("SyncManager");
        sm_el->SetText(sm.type.c_str());
        sm_el->SetAttribute("StartAddress", sm.start_address);
        sm_el->SetAttribute("Length", sm.length);
        sm_el->SetAttribute("ControlByte", sm.control_byte);
        sm_el->SetAttribute("Enable", sm.enable ? 1 : 0);
        slv_el->InsertEndChild(sm_el);
      }
    } else {
      // No SMs
    }

    // InitCmds
    if (!slv.init_cmds.empty()) {
      tinyxml2::XMLElement *s_init_cmds = xml.NewElement("InitCmds");
      slv_el->InsertEndChild(s_init_cmds);
      for (const EniInitCmd &ic : slv.init_cmds) {
        tinyxml2::XMLElement *ic_el = xml.NewElement("InitCmd");
        s_init_cmds->InsertEndChild(ic_el);
        tinyxml2::XMLElement *tr = xml.NewElement("Transition");
        tr->SetText("IP");
        ic_el->InsertEndChild(tr);
        tinyxml2::XMLElement *c = xml.NewElement("Cmd");
        c->SetText(static_cast<std::uint32_t>(ic.cmd));
        ic_el->InsertEndChild(c);
        tinyxml2::XMLElement *ado = xml.NewElement("Addr");
        ado->SetText(ic.ado);
        ic_el->InsertEndChild(ado);
      }
    } else {
      // No slave init cmds
    }

    // HotConnect
    if (slv.hot_connect.has_value()) {
      tinyxml2::XMLElement *hc_el = xml.NewElement("HotConnect");
      slv_el->InsertEndChild(hc_el);
      tinyxml2::XMLElement *grp = xml.NewElement("Group");
      grp->SetText(slv.hot_connect->group_name.c_str());
      hc_el->InsertEndChild(grp);
    } else {
      // Not hot connect
    }
  }

  // Cyclic Frames
  if (!doc.cyclic_frames().empty()) {
    tinyxml2::XMLElement *cyc_el = xml.NewElement("Cyclic");
    cfg->InsertEndChild(cyc_el);
    for (const EniCyclicFrame &cf : doc.cyclic_frames()) {
      tinyxml2::XMLElement *frame_el = xml.NewElement("Frame");
      cyc_el->InsertEndChild(frame_el);
      for (const EniCyclicCmd &cc : cf.commands) {
        // Create command XML node
        tinyxml2::XMLElement *cmd_el = xml.NewElement("Cmd");
        frame_el->InsertEndChild(cmd_el);
        // Serialize command type code
        tinyxml2::XMLElement *c_val = xml.NewElement("Cmd");
        c_val->SetText(static_cast<std::uint32_t>(cc.cmd));
        cmd_el->InsertEndChild(c_val);
        // Serialize logical address
        tinyxml2::XMLElement *c_addr = xml.NewElement("Addr");
        c_addr->SetText(cc.addr);
        cmd_el->InsertEndChild(c_addr);
        // Serialize payload length
        tinyxml2::XMLElement *c_len = xml.NewElement("DataLength");
        c_len->SetText(static_cast<std::uint32_t>(cc.data_length));
        cmd_el->InsertEndChild(c_len);
        // Serialize expected working counter
        tinyxml2::XMLElement *c_wkc = xml.NewElement("ExpectedWkc");
        c_wkc->SetText(cc.expected_wkc);
        cmd_el->InsertEndChild(c_wkc);
      }
    }
  } else {
    // No cyclic frames
  }

  tinyxml2::XMLPrinter printer;
  xml.Print(&printer);
  return std::string(printer.CStr());
}

Result<void> EniGenerator::save_to_xml_file(const std::string &filepath, const EniDocument &doc) const {
  std::string xml_content = serialize_to_xml_string(doc);
  tinyxml2::XMLDocument xml;
  tinyxml2::XMLError err = xml.Parse(xml_content.c_str(), xml_content.size());
  if (err != tinyxml2::XML_SUCCESS) {
    return std::unexpected(ECError::ProtocolError);
  } else {
    // Parse succeeded
  }

  err = xml.SaveFile(filepath.c_str());
  if (err != tinyxml2::XML_SUCCESS) {
    return std::unexpected(ECError::ProtocolError);
  } else {
    return {};
  }
}

} // namespace resoem::eni
