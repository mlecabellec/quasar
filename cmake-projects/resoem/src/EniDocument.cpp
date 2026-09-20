/**
 * @file EniDocument.cpp
 * @brief Implementation of ETG.2100 ENI AST and parser using tinyxml2.
 * @details Contributes to [FE-0040] and [TSK-20260920-001.1].
 */

#include "resoem/EniDocument.hpp"
#include "tinyxml2.h"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace resoem::eni {

namespace {

/**
 * @brief Parse integer string supporting decimal, 0x hex, #x hex, and negative values.
 */
std::uint32_t parse_num(const char *text, std::uint32_t default_val = 0) {
  if (text == nullptr) {
    return default_val;
  } else {
    // Valid text
  }
  std::string_view sv(text);
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
    sv.remove_prefix(1);
  }
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
    sv.remove_suffix(1);
  }
  if (sv.empty()) {
    return default_val;
  } else {
    // Non-empty view
  }

  bool is_negative = false;
  if (sv.front() == '-') {
    is_negative = true;
    sv.remove_prefix(1);
  } else if (sv.front() == '+') {
    sv.remove_prefix(1);
  } else {
    // Positive or unsigned
  }

  int base = 10;
  if (sv.starts_with("#x") || sv.starts_with("#X")) {
    base = 16;
    sv.remove_prefix(2);
  } else if (sv.starts_with("0x") || sv.starts_with("0X")) {
    base = 16;
    sv.remove_prefix(2);
  } else {
    base = 10;
  }

  std::uint64_t val = 0;
  std::from_chars_result res = std::from_chars(sv.data(), sv.data() + sv.size(), val, base);
  if (res.ec != std::errc()) {
    return default_val;
  } else {
    // Parsed
  }

  if (is_negative != false) {
    std::int64_t sval = -static_cast<std::int64_t>(val);
    return static_cast<std::uint32_t>(sval);
  } else {
    return static_cast<std::uint32_t>(val);
  }
}

/**
 * @brief Parse hex string into vector of bytes.
 */
std::vector<std::uint8_t> parse_hex_bytes(std::string_view hex_str) {
  std::vector<std::uint8_t> bytes;
  while (!hex_str.empty() && std::isspace(static_cast<unsigned char>(hex_str.front()))) {
    hex_str.remove_prefix(1);
  }
  while (!hex_str.empty() && std::isspace(static_cast<unsigned char>(hex_str.back()))) {
    hex_str.remove_suffix(1);
  }
  if (hex_str.starts_with("#x") || hex_str.starts_with("#X") ||
      hex_str.starts_with("0x") || hex_str.starts_with("0X")) {
    hex_str.remove_prefix(2);
  }

  std::size_t i = 0;
  const std::size_t STR_LEN = hex_str.size();
  bytes.reserve(STR_LEN / 2);
  while (i < STR_LEN) {
    if (i + 1 >= STR_LEN) {
      // Single trailing hex nibble
      char buf[2] = {hex_str[i], '\0'};
      std::uint8_t byte_val = 0;
      std::from_chars(buf, buf + 1, byte_val, 16);
      bytes.push_back(byte_val);
      i += 1;
    } else {
      char buf[3] = {hex_str[i], hex_str[i + 1], '\0'};
      std::uint8_t byte_val = 0;
      std::from_chars(buf, buf + 2, byte_val, 16);
      bytes.push_back(byte_val);
      i += 2;
    }
  }
  return bytes;
}

/**
 * @brief Parse string transition to EsmTransition enum.
 */
EsmTransition parse_transition(std::string_view sv) {
  if (sv == "IP") {
    // Init to Pre-Operational
    return EsmTransition::IP;
  } else if (sv == "PS") {
    // Pre-Operational to Safe-Operational
    return EsmTransition::PS;
  } else if (sv == "PI") {
    // Pre-Operational to Init
    return EsmTransition::PI;
  } else if (sv == "SP") {
    // Safe-Operational to Pre-Operational
    return EsmTransition::SP;
  } else if (sv == "SO") {
    // Safe-Operational to Operational
    return EsmTransition::SO;
  } else if (sv == "SI") {
    // Safe-Operational to Init
    return EsmTransition::SI;
  } else if (sv == "OS") {
    // Operational to Safe-Operational
    return EsmTransition::OS;
  } else if (sv == "OP") {
    // Operational to Pre-Operational
    return EsmTransition::OP;
  } else if (sv == "OI") {
    // Operational to Init
    return EsmTransition::OI;
  } else if (sv == "IB") {
    // Init to Bootstrap
    return EsmTransition::IB;
  } else if (sv == "BI") {
    // Bootstrap to Init
    return EsmTransition::BI;
  } else if (sv == "II") {
    // Init to Init
    return EsmTransition::II;
  } else if (sv == "PP") {
    // Pre-Operational to Pre-Operational
    return EsmTransition::PP;
  } else if (sv == "SS") {
    // Safe-Operational to Safe-Operational
    return EsmTransition::SS;
  } else {
    // Unrecognized transition code
    return EsmTransition::NONE;
  }
}

/**
 * @brief Parse InitCmd element.
 */
EniInitCmd parse_init_cmd_elem(const tinyxml2::XMLElement *elem) {
  EniInitCmd cmd;
  const tinyxml2::XMLElement *trans_elem = elem->FirstChildElement("Transition");
  while (trans_elem != nullptr) {
    if (trans_elem->GetText() != nullptr) {
      cmd.transition = cmd.transition | parse_transition(trans_elem->GetText());
    }
    trans_elem = trans_elem->NextSiblingElement("Transition");
  }

  const tinyxml2::XMLElement *before_elem = elem->FirstChildElement("BeforeSlave");
  if (before_elem != nullptr && before_elem->GetText() != nullptr) {
    std::string_view sv(before_elem->GetText());
    cmd.before_slave = (sv == "true" || sv == "1");
  } else {
    cmd.before_slave = false;
  }

  const tinyxml2::XMLElement *comment_elem = elem->FirstChildElement("Comment");
  if (comment_elem != nullptr && comment_elem->GetText() != nullptr) {
    cmd.comment = comment_elem->GetText();
  }

  const tinyxml2::XMLElement *req_elem = elem->FirstChildElement("Requires");
  if (req_elem != nullptr && req_elem->GetText() != nullptr) {
    cmd.requirement = req_elem->GetText();
  }

  const tinyxml2::XMLElement *cmd_code_elem = elem->FirstChildElement("Cmd");
  if (cmd_code_elem != nullptr) {
    std::uint32_t code = parse_num(cmd_code_elem->GetText());
    cmd.cmd = static_cast<DatagramCmd>(code);
  }

  const tinyxml2::XMLElement *adp_elem = elem->FirstChildElement("Adp");
  if (adp_elem != nullptr) {
    cmd.adp = static_cast<std::uint16_t>(parse_num(adp_elem->GetText()));
  }

  const tinyxml2::XMLElement *ado_elem = elem->FirstChildElement("Ado");
  if (ado_elem != nullptr) {
    cmd.ado = static_cast<std::uint16_t>(parse_num(ado_elem->GetText()));
  }

  const tinyxml2::XMLElement *addr_elem = elem->FirstChildElement("Addr");
  if (addr_elem != nullptr) {
    cmd.addr = parse_num(addr_elem->GetText());
  }

  const tinyxml2::XMLElement *data_elem = elem->FirstChildElement("Data");
  if (data_elem != nullptr && data_elem->GetText() != nullptr) {
    cmd.data = parse_hex_bytes(data_elem->GetText());
    cmd.data_length = cmd.data.size();
  }

  const tinyxml2::XMLElement *len_elem = elem->FirstChildElement("DataLength");
  if (len_elem != nullptr) {
    cmd.data_length = parse_num(len_elem->GetText());
  }

  const tinyxml2::XMLElement *retries_elem = elem->FirstChildElement("Retries");
  if (retries_elem != nullptr) {
    cmd.retries = parse_num(retries_elem->GetText(), DEFAULT_INIT_CMD_RETRIES);
  }

  const tinyxml2::XMLElement *to_elem = elem->FirstChildElement("Timeout");
  if (to_elem != nullptr) {
    cmd.timeout_ms = parse_num(to_elem->GetText(), DEFAULT_INIT_CMD_TIMEOUT_MS);
  }

  const tinyxml2::XMLElement *val_elem = elem->FirstChildElement("Validate");
  if (val_elem != nullptr) {
    ValidateRule rule;
    const char *signed_attr = val_elem->Attribute("Signed");
    if (signed_attr != nullptr) {
      std::string_view ssv(signed_attr);
      rule.signed_val = (ssv == "true" || ssv == "1");
    }
    const char *type_attr = val_elem->Attribute("Type");
    if (type_attr != nullptr) {
      rule.type = type_attr;
    }
    const tinyxml2::XMLElement *vdata = val_elem->FirstChildElement("Data");
    if (vdata != nullptr && vdata->GetText() != nullptr) {
      rule.data = parse_hex_bytes(vdata->GetText());
    }
    const tinyxml2::XMLElement *vmask = val_elem->FirstChildElement("DataMask");
    if (vmask != nullptr && vmask->GetText() != nullptr) {
      rule.data_mask = parse_hex_bytes(vmask->GetText());
    }
    const tinyxml2::XMLElement *vto = val_elem->FirstChildElement("Timeout");
    if (vto != nullptr) {
      rule.timeout_ms = parse_num(vto->GetText(), DEFAULT_INIT_CMD_TIMEOUT_MS);
    }
    cmd.validate = std::move(rule);
  }

  return cmd;
}

// Internal element parsing helpers
void parse_master_node(const tinyxml2::XMLElement *config_elem, EniMaster &master) {
  const tinyxml2::XMLElement *master_elem = config_elem->FirstChildElement("Master");
  if (master_elem != nullptr) {
    const tinyxml2::XMLElement *info_elem = master_elem->FirstChildElement("Info");
    if (info_elem != nullptr) {
      const tinyxml2::XMLElement *name_e = info_elem->FirstChildElement("Name");
      if (name_e != nullptr && name_e->GetText() != nullptr) {
        master.name = name_e->GetText();
      } else {
        // No master name
      }
      const tinyxml2::XMLElement *dst_e = info_elem->FirstChildElement("Destination");
      if (dst_e != nullptr && dst_e->GetText() != nullptr) {
        master.destination_mac = dst_e->GetText();
      } else {
        // No destination MAC
      }
      const tinyxml2::XMLElement *src_e = info_elem->FirstChildElement("Source");
      if (src_e != nullptr && src_e->GetText() != nullptr) {
        master.source_mac = src_e->GetText();
      } else {
        // No source MAC
      }
      const tinyxml2::XMLElement *eth_e = info_elem->FirstChildElement("EtherType");
      if (eth_e != nullptr) {
        master.ethertype = static_cast<std::uint16_t>(parse_num(eth_e->GetText(), ETHERCAT_ETHERTYPE_VAL));
      } else {
        // Default ethertype
      }
    } else {
      // No info element
    }

    const tinyxml2::XMLElement *minit_cmds = master_elem->FirstChildElement("InitCmds");
    if (minit_cmds != nullptr) {
      const tinyxml2::XMLElement *ic_elem = minit_cmds->FirstChildElement("InitCmd");
      std::size_t cmd_count = 0;
      while (ic_elem != nullptr && cmd_count < MAX_INIT_CMDS_PER_SLAVE) {
        master.init_cmds.push_back(parse_init_cmd_elem(ic_elem));
        cmd_count += 1;
        ic_elem = ic_elem->NextSiblingElement("InitCmd");
      }
    } else {
      // No master init cmds
    }
  } else {
    // No master element
  }
}

void parse_slave_process_data(const tinyxml2::XMLElement *pd_elem, EniSlave &slave) {
  if (pd_elem == nullptr) {
    return;
  } else {
    // ProcessData present
  }

  for (int sm_idx = 0; sm_idx < 4; ++sm_idx) {
    std::string sm_tag = "Sm" + std::to_string(sm_idx);
    const tinyxml2::XMLElement *sme = pd_elem->FirstChildElement(sm_tag.c_str());
    if (sme != nullptr) {
      EniSyncManager sm;
      sm.index = static_cast<std::uint8_t>(sm_idx);
      const tinyxml2::XMLElement *te = sme->FirstChildElement("Type");
      if (te != nullptr && te->GetText() != nullptr) {
        sm.type = te->GetText();
      } else {
        // No type
      }
      const tinyxml2::XMLElement *sa = sme->FirstChildElement("StartAddress");
      if (sa != nullptr) {
        sm.start_address = static_cast<std::uint16_t>(parse_num(sa->GetText()));
      } else {
        // No start address
      }
      const tinyxml2::XMLElement *cb = sme->FirstChildElement("ControlByte");
      if (cb != nullptr) {
        sm.control_byte = static_cast<std::uint8_t>(parse_num(cb->GetText()));
      } else {
        // No control byte
      }
      const tinyxml2::XMLElement *ene = sme->FirstChildElement("Enable");
      if (ene != nullptr && ene->GetText() != nullptr) {
        std::string_view sv(ene->GetText());
        sm.enable = (sv == "true" || sv == "1");
      } else {
        sm.enable = false;
      }
      slave.sync_managers.push_back(std::move(sm));
    } else {
      // SM not present
    }
  }

  // RxPdo
  const tinyxml2::XMLElement *rx_pdo = pd_elem->FirstChildElement("RxPdo");
  while (rx_pdo != nullptr) {
    EniPdo pdo;
    pdo.is_tx = false;
    const tinyxml2::XMLElement *idx_e = rx_pdo->FirstChildElement("Index");
    if (idx_e != nullptr) {
      pdo.index = static_cast<std::uint16_t>(parse_num(idx_e->GetText()));
    } else {
      // No index
    }
    const tinyxml2::XMLElement *nm_e = rx_pdo->FirstChildElement("Name");
    if (nm_e != nullptr && nm_e->GetText() != nullptr) {
      pdo.name = nm_e->GetText();
    } else {
      // No name
    }
    const char *sm_attr = rx_pdo->Attribute("Sm");
    if (sm_attr != nullptr) {
      pdo.sync_manager = static_cast<std::uint8_t>(parse_num(sm_attr));
    } else {
      // No SM attr
    }
    const char *mand_attr = rx_pdo->Attribute("Mandatory");
    if (mand_attr != nullptr) {
      std::string_view sv(mand_attr);
      pdo.mandatory = (sv == "true" || sv == "1");
    } else {
      pdo.mandatory = false;
    }

    const tinyxml2::XMLElement *ent_e = rx_pdo->FirstChildElement("Entry");
    while (ent_e != nullptr) {
      EniPdoEntry entry;
      const tinyxml2::XMLElement *ei = ent_e->FirstChildElement("Index");
      if (ei != nullptr) {
        entry.index = static_cast<std::uint16_t>(parse_num(ei->GetText()));
      } else {
        // No index
      }
      const tinyxml2::XMLElement *esi = ent_e->FirstChildElement("SubIndex");
      if (esi != nullptr) {
        entry.subindex = static_cast<std::uint8_t>(parse_num(esi->GetText()));
      } else {
        // No subindex
      }
      const tinyxml2::XMLElement *ebl = ent_e->FirstChildElement("BitLen");
      if (ebl != nullptr) {
        entry.bit_len = static_cast<std::uint8_t>(parse_num(ebl->GetText()));
      } else {
        // No bit length
      }
      const tinyxml2::XMLElement *enm = ent_e->FirstChildElement("Name");
      if (enm != nullptr && enm->GetText() != nullptr) {
        entry.name = enm->GetText();
      } else {
        // No entry name
      }
      const tinyxml2::XMLElement *edt = ent_e->FirstChildElement("DataType");
      if (edt != nullptr && edt->GetText() != nullptr) {
        entry.data_type = edt->GetText();
      } else {
        // No data type
      }
      pdo.entries.push_back(std::move(entry));
      ent_e = ent_e->NextSiblingElement("Entry");
    }
    slave.rx_pdos.push_back(std::move(pdo));
    rx_pdo = rx_pdo->NextSiblingElement("RxPdo");
  }

  // TxPdo
  const tinyxml2::XMLElement *tx_pdo = pd_elem->FirstChildElement("TxPdo");
  while (tx_pdo != nullptr) {
    EniPdo pdo;
    pdo.is_tx = true;
    const tinyxml2::XMLElement *idx_e = tx_pdo->FirstChildElement("Index");
    if (idx_e != nullptr) {
      pdo.index = static_cast<std::uint16_t>(parse_num(idx_e->GetText()));
    } else {
      // No index
    }
    const tinyxml2::XMLElement *nm_e = tx_pdo->FirstChildElement("Name");
    if (nm_e != nullptr && nm_e->GetText() != nullptr) {
      pdo.name = nm_e->GetText();
    } else {
      // No name
    }
    const char *sm_attr = tx_pdo->Attribute("Sm");
    if (sm_attr != nullptr) {
      pdo.sync_manager = static_cast<std::uint8_t>(parse_num(sm_attr));
    } else {
      // No SM attr
    }
    const char *mand_attr = tx_pdo->Attribute("Mandatory");
    if (mand_attr != nullptr) {
      std::string_view sv(mand_attr);
      pdo.mandatory = (sv == "true" || sv == "1");
    } else {
      pdo.mandatory = false;
    }

    const tinyxml2::XMLElement *ent_e = tx_pdo->FirstChildElement("Entry");
    while (ent_e != nullptr) {
      EniPdoEntry entry;
      const tinyxml2::XMLElement *ei = ent_e->FirstChildElement("Index");
      if (ei != nullptr) {
        entry.index = static_cast<std::uint16_t>(parse_num(ei->GetText()));
      } else {
        // No index
      }
      const tinyxml2::XMLElement *esi = ent_e->FirstChildElement("SubIndex");
      if (esi != nullptr) {
        entry.subindex = static_cast<std::uint8_t>(parse_num(esi->GetText()));
      } else {
        // No subindex
      }
      const tinyxml2::XMLElement *ebl = ent_e->FirstChildElement("BitLen");
      if (ebl != nullptr) {
        entry.bit_len = static_cast<std::uint8_t>(parse_num(ebl->GetText()));
      } else {
        // No bit length
      }
      const tinyxml2::XMLElement *enm = ent_e->FirstChildElement("Name");
      if (enm != nullptr && enm->GetText() != nullptr) {
        entry.name = enm->GetText();
      } else {
        // No entry name
      }
      const tinyxml2::XMLElement *edt = ent_e->FirstChildElement("DataType");
      if (edt != nullptr && edt->GetText() != nullptr) {
        entry.data_type = edt->GetText();
      } else {
        // No data type
      }
      pdo.entries.push_back(std::move(entry));
      ent_e = ent_e->NextSiblingElement("Entry");
    }
    slave.tx_pdos.push_back(std::move(pdo));
    tx_pdo = tx_pdo->NextSiblingElement("TxPdo");
  }
}

void parse_slave_mailbox(const tinyxml2::XMLElement *mb_elem, EniSlave &slave) {
  if (mb_elem == nullptr) {
    return;
  } else {
    // Mailbox present
  }
  const char *dll_attr = mb_elem->Attribute("DataLinkLayer");
  if (dll_attr != nullptr) {
    std::string_view sv(dll_attr);
    slave.mailbox.data_link_layer = (sv == "true" || sv == "1");
  } else {
    slave.mailbox.data_link_layer = true;
  }
  const tinyxml2::XMLElement *s_elem = mb_elem->FirstChildElement("Send");
  if (s_elem != nullptr) {
    const tinyxml2::XMLElement *st = s_elem->FirstChildElement("Start");
    if (st != nullptr) {
      slave.mailbox.send_start = static_cast<std::uint16_t>(parse_num(st->GetText()));
    } else {
      // No start
    }
    const tinyxml2::XMLElement *ln = s_elem->FirstChildElement("Length");
    if (ln != nullptr) {
      slave.mailbox.send_length = static_cast<std::uint16_t>(parse_num(ln->GetText()));
    } else {
      // No length
    }
  } else {
    // No send elem
  }
  const tinyxml2::XMLElement *r_elem = mb_elem->FirstChildElement("Receive");
  if (r_elem != nullptr) {
    const tinyxml2::XMLElement *st = r_elem->FirstChildElement("Start");
    if (st != nullptr) {
      slave.mailbox.recv_start = static_cast<std::uint16_t>(parse_num(st->GetText()));
    } else {
      // No start
    }
    const tinyxml2::XMLElement *ln = r_elem->FirstChildElement("Length");
    if (ln != nullptr) {
      slave.mailbox.recv_length = static_cast<std::uint16_t>(parse_num(ln->GetText()));
    } else {
      // No length
    }
    const tinyxml2::XMLElement *sb = r_elem->FirstChildElement("StatusBitAddr");
    if (sb != nullptr) {
      slave.mailbox.status_bit_addr = static_cast<std::uint16_t>(parse_num(sb->GetText()));
    } else {
      // No status bit addr
    }
  } else {
    // No receive elem
  }

  const tinyxml2::XMLElement *proto_e = mb_elem->FirstChildElement("Protocol");
  while (proto_e != nullptr) {
    if (proto_e->GetText() != nullptr) {
      slave.mailbox.protocols.push_back(proto_e->GetText());
    } else {
      // Empty protocol string
    }
    proto_e = proto_e->NextSiblingElement("Protocol");
  }

  const tinyxml2::XMLElement *coe_e = mb_elem->FirstChildElement("CoE");
  if (coe_e != nullptr) {
    const tinyxml2::XMLElement *coe_ics = coe_e->FirstChildElement("InitCmds");
    if (coe_ics != nullptr) {
      const tinyxml2::XMLElement *cic = coe_ics->FirstChildElement("InitCmd");
      while (cic != nullptr) {
        EniCoEInitCmd c;
        const tinyxml2::XMLElement *tr = cic->FirstChildElement("Transition");
        if (tr != nullptr && tr->GetText() != nullptr) {
          c.transition = parse_transition(tr->GetText());
        } else {
          // No transition
        }
        const tinyxml2::XMLElement *cm = cic->FirstChildElement("Comment");
        if (cm != nullptr && cm->GetText() != nullptr) {
          c.comment = cm->GetText();
        } else {
          // No comment
        }
        const tinyxml2::XMLElement *idx = cic->FirstChildElement("Index");
        if (idx != nullptr) {
          c.index = static_cast<std::uint16_t>(parse_num(idx->GetText()));
        } else {
          // No index
        }
        const tinyxml2::XMLElement *sidx = cic->FirstChildElement("SubIndex");
        if (sidx != nullptr) {
          c.subindex = static_cast<std::uint8_t>(parse_num(sidx->GetText()));
        } else {
          // No subindex
        }
        const tinyxml2::XMLElement *ccs = cic->FirstChildElement("Ccs");
        if (ccs != nullptr) {
          c.ccs = static_cast<std::uint8_t>(parse_num(ccs->GetText(), 1));
        } else {
          c.ccs = 1;
        }
        const char *ca_attr = cic->Attribute("CompleteAccess");
        if (ca_attr != nullptr) {
          std::string_view sv(ca_attr);
          c.complete_access = (sv == "true" || sv == "1");
        } else {
          c.complete_access = false;
        }
        const char *fix_attr = cic->Attribute("Fixed");
        if (fix_attr != nullptr) {
          std::string_view sv(fix_attr);
          c.fixed = (sv == "true" || sv == "1");
        } else {
          c.fixed = false;
        }
        const tinyxml2::XMLElement *dt = cic->FirstChildElement("Data");
        if (dt != nullptr && dt->GetText() != nullptr) {
          c.data = parse_hex_bytes(dt->GetText());
        } else {
          // No data
        }
        slave.mailbox.coe_init_cmds.push_back(std::move(c));
        cic = cic->NextSiblingElement("InitCmd");
      }
    } else {
      // No coe init cmds
    }
  } else {
    // No CoE
  }
}

void parse_slave_section(const tinyxml2::XMLElement *slave_elem, EniSlave &slave) {
  const tinyxml2::XMLElement *sinfo = slave_elem->FirstChildElement("Info");
  if (sinfo != nullptr) {
    const tinyxml2::XMLElement *sn = sinfo->FirstChildElement("Name");
    if (sn != nullptr && sn->GetText() != nullptr) {
      slave.name = sn->GetText();
    } else {
      // No slave name
    }
    const tinyxml2::XMLElement *pa = sinfo->FirstChildElement("PhysAddr");
    if (pa != nullptr) {
      slave.phys_addr = static_cast<std::uint16_t>(parse_num(pa->GetText()));
    } else {
      // No phys addr
    }
    const tinyxml2::XMLElement *aia = sinfo->FirstChildElement("AutoIncAddr");
    if (aia != nullptr) {
      slave.auto_inc_addr = static_cast<std::int16_t>(parse_num(aia->GetText()));
    } else {
      // No auto inc addr
    }
    const tinyxml2::XMLElement *phy = sinfo->FirstChildElement("Physics");
    if (phy != nullptr && phy->GetText() != nullptr) {
      slave.physics = phy->GetText();
    } else {
      // No physics
    }
    const tinyxml2::XMLElement *vi = sinfo->FirstChildElement("VendorId");
    if (vi != nullptr) {
      slave.vendor_id = parse_num(vi->GetText());
    } else {
      // No vendor id
    }
    const tinyxml2::XMLElement *pc = sinfo->FirstChildElement("ProductCode");
    if (pc != nullptr) {
      slave.product_code = parse_num(pc->GetText());
    } else {
      // No product code
    }
    const tinyxml2::XMLElement *rn = sinfo->FirstChildElement("RevisionNo");
    if (rn != nullptr) {
      slave.revision_no = parse_num(rn->GetText());
    } else {
      // No revision no
    }
    const tinyxml2::XMLElement *sn_elem = sinfo->FirstChildElement("SerialNo");
    if (sn_elem != nullptr) {
      slave.serial_no = parse_num(sn_elem->GetText());
    } else {
      // No serial no
    }
  } else {
    // No info element
  }

  // PreviousPort
  const tinyxml2::XMLElement *pp_elem = slave_elem->FirstChildElement("PreviousPort");
  if (pp_elem != nullptr) {
    EniPreviousPort pp;
    const char *sel_attr = pp_elem->Attribute("Selected");
    if (sel_attr != nullptr) {
      std::string_view ssv(sel_attr);
      pp.selected = (ssv == "true" || ssv == "1");
    } else {
      pp.selected = true;
    }
    const tinyxml2::XMLElement *porte = pp_elem->FirstChildElement("Port");
    if (porte != nullptr && porte->GetText() != nullptr && porte->GetText()[0] != '\0') {
      pp.port = porte->GetText()[0];
    } else {
      pp.port = 'A';
    }
    const tinyxml2::XMLElement *ppa = pp_elem->FirstChildElement("PhysAddr");
    if (ppa != nullptr) {
      pp.phys_addr = static_cast<std::uint16_t>(parse_num(ppa->GetText()));
    } else {
      // No parent phys addr
    }
    slave.previous_port = pp;
  } else {
    // No previous port
  }

  // HotConnect
  const tinyxml2::XMLElement *hc_elem = slave_elem->FirstChildElement("HotConnect");
  if (hc_elem != nullptr) {
    EniHotConnect hc;
    const tinyxml2::XMLElement *gn = hc_elem->FirstChildElement("GroupName");
    if (gn != nullptr && gn->GetText() != nullptr) {
      hc.group_name = gn->GetText();
    } else {
      // No group name
    }
    const tinyxml2::XMLElement *gmc = hc_elem->FirstChildElement("GroupMemberCnt");
    if (gmc != nullptr) {
      hc.group_member_count = parse_num(gmc->GetText(), 1);
    } else {
      hc.group_member_count = 1;
    }
    const tinyxml2::XMLElement *id_cmd = hc_elem->FirstChildElement("IdentifyCmd");
    while (id_cmd != nullptr) {
      hc.identify_cmds.push_back(parse_init_cmd_elem(id_cmd));
      id_cmd = id_cmd->NextSiblingElement("IdentifyCmd");
    }
    slave.hot_connect = std::move(hc);
  } else {
    // No hot connect
  }

  // ProcessData
  const tinyxml2::XMLElement *pd_elem = slave_elem->FirstChildElement("ProcessData");
  parse_slave_process_data(pd_elem, slave);

  // Mailbox
  const tinyxml2::XMLElement *mb_elem = slave_elem->FirstChildElement("Mailbox");
  parse_slave_mailbox(mb_elem, slave);

  // InitCmds (Register level)
  const tinyxml2::XMLElement *sinit_cmds = slave_elem->FirstChildElement("InitCmds");
  if (sinit_cmds != nullptr) {
    const tinyxml2::XMLElement *ic_elem = sinit_cmds->FirstChildElement("InitCmd");
    std::size_t count = 0;
    while (ic_elem != nullptr && count < MAX_INIT_CMDS_PER_SLAVE) {
      slave.init_cmds.push_back(parse_init_cmd_elem(ic_elem));
      count += 1;
      ic_elem = ic_elem->NextSiblingElement("InitCmd");
    }
  } else {
    // No init cmds
  }

  // DC
  const tinyxml2::XMLElement *dc_elem = slave_elem->FirstChildElement("DC");
  if (dc_elem != nullptr) {
    slave.dc.enabled = true;
    const tinyxml2::XMLElement *prc = dc_elem->FirstChildElement("PotentialReferenceClock");
    if (prc != nullptr && prc->GetText() != nullptr) {
      std::string_view sv(prc->GetText());
      slave.dc.potential_ref_clock = (sv == "true" || sv == "1");
    } else {
      slave.dc.potential_ref_clock = false;
    }
    const tinyxml2::XMLElement *ct0 = dc_elem->FirstChildElement("CycleTime0");
    if (ct0 != nullptr) {
      slave.dc.cycle_time_0 = parse_num(ct0->GetText());
    } else {
      // No cycle time 0
    }
    const tinyxml2::XMLElement *ct1 = dc_elem->FirstChildElement("CycleTime1");
    if (ct1 != nullptr) {
      slave.dc.cycle_time_1 = parse_num(ct1->GetText());
    } else {
      // No cycle time 1
    }
    const tinyxml2::XMLElement *st0 = dc_elem->FirstChildElement("ShiftTime0");
    if (st0 != nullptr) {
      slave.dc.shift_time_0 = static_cast<std::int32_t>(parse_num(st0->GetText()));
    } else {
      // No shift time 0
    }
    const tinyxml2::XMLElement *st1 = dc_elem->FirstChildElement("ShiftTime1");
    if (st1 != nullptr) {
      slave.dc.shift_time_1 = static_cast<std::int32_t>(parse_num(st1->GetText()));
    } else {
      // No shift time 1
    }
  } else {
    // No DC
  }
}

void parse_cyclic_section(const tinyxml2::XMLElement *config_elem, std::uint32_t &cycle_time_us,
                          std::vector<EniCyclicFrame> &cyclic_frames) {
  cyclic_frames.clear();
  const tinyxml2::XMLElement *cyclic_elem = config_elem->FirstChildElement("Cyclic");
  if (cyclic_elem != nullptr) {
    const tinyxml2::XMLElement *ct_elem = cyclic_elem->FirstChildElement("CycleTime");
    if (ct_elem != nullptr) {
      cycle_time_us = parse_num(ct_elem->GetText(), DEFAULT_CYCLE_TIME_US);
    } else {
      // Default cycle time
    }
    const tinyxml2::XMLElement *frame_elem = cyclic_elem->FirstChildElement("Frame");
    while (frame_elem != nullptr) {
      EniCyclicFrame frame;
      const tinyxml2::XMLElement *cmd_elem = frame_elem->FirstChildElement("Cmd");
      std::size_t cmd_idx = 0;
      while (cmd_elem != nullptr && cmd_idx < MAX_CYCLIC_CMDS) {
        EniCyclicCmd cmd;
        const tinyxml2::XMLElement *ste = cmd_elem->FirstChildElement("State");
        while (ste != nullptr) {
          if (ste->GetText() != nullptr) {
            cmd.states.push_back(ste->GetText());
          } else {
            // No state
          }
          ste = ste->NextSiblingElement("State");
        }
        const tinyxml2::XMLElement *cm = cmd_elem->FirstChildElement("Comment");
        if (cm != nullptr && cm->GetText() != nullptr) {
          cmd.comment = cm->GetText();
        } else {
          // No comment
        }
        const tinyxml2::XMLElement *cde = cmd_elem->FirstChildElement("Cmd");
        if (cde != nullptr) {
          cmd.cmd = static_cast<DatagramCmd>(parse_num(cde->GetText()));
        } else {
          // No cmd code
        }
        const tinyxml2::XMLElement *addr = cmd_elem->FirstChildElement("Addr");
        if (addr != nullptr) {
          cmd.addr = parse_num(addr->GetText());
        } else {
          // No addr
        }
        const tinyxml2::XMLElement *adp = cmd_elem->FirstChildElement("Adp");
        if (adp != nullptr) {
          cmd.adp = static_cast<std::uint16_t>(parse_num(adp->GetText()));
        } else {
          // No adp
        }
        const tinyxml2::XMLElement *ado = cmd_elem->FirstChildElement("Ado");
        if (ado != nullptr) {
          cmd.ado = static_cast<std::uint16_t>(parse_num(ado->GetText()));
        } else {
          // No ado
        }
        const tinyxml2::XMLElement *dl = cmd_elem->FirstChildElement("DataLength");
        if (dl != nullptr) {
          cmd.data_length = parse_num(dl->GetText());
        } else {
          // No data length
        }
        const tinyxml2::XMLElement *cnt = cmd_elem->FirstChildElement("Cnt");
        if (cnt != nullptr) {
          cmd.expected_wkc = static_cast<std::uint16_t>(parse_num(cnt->GetText()));
        } else {
          // No expected wkc
        }
        const tinyxml2::XMLElement *io = cmd_elem->FirstChildElement("InputOffs");
        if (io != nullptr) {
          cmd.input_offset = parse_num(io->GetText());
        } else {
          // No input offset
        }
        const tinyxml2::XMLElement *oo = cmd_elem->FirstChildElement("OutputOffs");
        if (oo != nullptr) {
          cmd.output_offset = parse_num(oo->GetText());
        } else {
          // No output offset
        }
        frame.commands.push_back(std::move(cmd));
        cmd_idx += 1;
        cmd_elem = cmd_elem->NextSiblingElement("Cmd");
      }
      cyclic_frames.push_back(std::move(frame));
      frame_elem = frame_elem->NextSiblingElement("Frame");
    }
  } else {
    // No cyclic section
  }
}

void parse_process_image_section(const tinyxml2::XMLElement *config_elem,
                                 std::vector<EniProcessImageVariable> &process_image_variables) {
  process_image_variables.clear();
  const tinyxml2::XMLElement *pi_elem = config_elem->FirstChildElement("ProcessImage");
  if (pi_elem != nullptr) {
    // Parse Inputs process image entries
    const tinyxml2::XMLElement *inp_elem = pi_elem->FirstChildElement("Inputs");
    if (inp_elem != nullptr) {
      const tinyxml2::XMLElement *var_elem = inp_elem->FirstChildElement("Variable");
      std::size_t var_idx = 0;
      while (var_elem != nullptr && var_idx < MAX_PROCESS_IMAGE_VARIABLES) {
        EniProcessImageVariable var;
        var.is_input = true;
        // Parse variable name
        const tinyxml2::XMLElement *vn = var_elem->FirstChildElement("Name");
        if (vn != nullptr && vn->GetText() != nullptr) {
          var.name = vn->GetText();
        } else {
          // No name
        }
        // Parse data type
        const tinyxml2::XMLElement *vdt = var_elem->FirstChildElement("DataType");
        if (vdt != nullptr && vdt->GetText() != nullptr) {
          var.data_type = vdt->GetText();
        } else {
          // No data type
        }
        // Parse bit size
        const tinyxml2::XMLElement *vbs = var_elem->FirstChildElement("BitSize");
        if (vbs != nullptr) {
          var.bit_size = parse_num(vbs->GetText());
        } else {
          // No bit size
        }
        // Parse bit offset
        const tinyxml2::XMLElement *vbo = var_elem->FirstChildElement("BitOffs");
        if (vbo != nullptr) {
          var.bit_offset = parse_num(vbo->GetText());
        } else {
          // No bit offset
        }
        process_image_variables.push_back(std::move(var));
        var_idx += 1;
        var_elem = var_elem->NextSiblingElement("Variable");
      }
    } else {
      // No inputs
    }

    const tinyxml2::XMLElement *out_elem = pi_elem->FirstChildElement("Outputs");
    if (out_elem != nullptr) {
      const tinyxml2::XMLElement *var_elem = out_elem->FirstChildElement("Variable");
      std::size_t var_idx = 0;
      while (var_elem != nullptr && var_idx < MAX_PROCESS_IMAGE_VARIABLES) {
        EniProcessImageVariable var;
        var.is_input = false;
        const tinyxml2::XMLElement *vn = var_elem->FirstChildElement("Name");
        if (vn != nullptr && vn->GetText() != nullptr) {
          var.name = vn->GetText();
        } else {
          // No name
        }
        const tinyxml2::XMLElement *vdt = var_elem->FirstChildElement("DataType");
        if (vdt != nullptr && vdt->GetText() != nullptr) {
          var.data_type = vdt->GetText();
        } else {
          // No data type
        }
        const tinyxml2::XMLElement *vbs = var_elem->FirstChildElement("BitSize");
        if (vbs != nullptr) {
          var.bit_size = parse_num(vbs->GetText());
        } else {
          // No bit size
        }
        const tinyxml2::XMLElement *vbo = var_elem->FirstChildElement("BitOffs");
        if (vbo != nullptr) {
          var.bit_offset = parse_num(vbo->GetText());
        } else {
          // No bit offset
        }
        process_image_variables.push_back(std::move(var));
        var_idx += 1;
        var_elem = var_elem->NextSiblingElement("Variable");
      }
    } else {
      // No outputs
    }
  } else {
    // No process image
  }
}

} // anonymous namespace

Result<> EniDocument::load_from_file(const std::string &filepath) {
  std::ifstream f(filepath, std::ios::in | std::ios::binary);
  if (!f.is_open()) {
    return std::unexpected(ECError::ProtocolError);
  } else {
    // File opened
  }

  f.seekg(0, std::ios::end);
  const std::size_t FILE_SIZE = static_cast<std::size_t>(f.tellg());
  f.seekg(0, std::ios::beg);

  if (FILE_SIZE > MAX_XML_FILE_SIZE_BYTES) {
    return std::unexpected(ECError::ProtocolError);
  } else {
    // Within file size limit
  }

  std::string buffer;
  buffer.resize(FILE_SIZE);
  f.read(buffer.data(), static_cast<std::streamsize>(FILE_SIZE));
  return load_from_memory(buffer);
}

Result<> EniDocument::load_from_memory(std::string_view xml_content) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError err = doc.Parse(xml_content.data(), xml_content.size());
  if (err != tinyxml2::XML_SUCCESS) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid XML syntax
  }

  const tinyxml2::XMLElement *root = doc.RootElement();
  if (root == nullptr) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Root element found
  }

  std::string_view root_name(root->Name());
  const tinyxml2::XMLElement *config_elem = nullptr;
  if (root_name == "EtherCATConfig") {
    config_elem = root->FirstChildElement("Config");
    const char *ver = root->Attribute("Version");
    if (ver != nullptr) {
      version_ = ver;
    } else {
      // Retain default version
    }
  } else if (root_name == "Config") {
    config_elem = root;
  } else {
    return std::unexpected(ECError::InvalidResponse);
  }

  if (config_elem == nullptr) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid config element
  }

  // Parse Master
  parse_master_node(config_elem, master_);

  // Parse Slaves
  slaves_.clear();
  const tinyxml2::XMLElement *slave_elem = config_elem->FirstChildElement("Slave");
  std::size_t slave_idx = 0;
  while (slave_elem != nullptr && slave_idx < MAX_ENI_SLAVES) {
    EniSlave slave;
    parse_slave_section(slave_elem, slave);
    slaves_.push_back(std::move(slave));
    slave_idx += 1;
    slave_elem = slave_elem->NextSiblingElement("Slave");
  }

  // Parse Cyclic frames
  parse_cyclic_section(config_elem, cycle_time_us_, cyclic_frames_);

  // Parse ProcessImage variables
  parse_process_image_section(config_elem, process_image_variables_);

  return {};
}

Result<> EniDocument::save_to_file(const std::string &filepath) const {
  Result<std::string> xml_res = save_to_string();
  if (!xml_res) {
    return std::unexpected(xml_res.error());
  }

  std::ofstream out(filepath, std::ios::out | std::ios::trunc);
  if (!out.is_open()) {
    return std::unexpected(ECError::ProtocolError);
  }
  out << *xml_res;
  return {};
}

Result<std::string> EniDocument::save_to_string() const {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLDeclaration *decl = doc.NewDeclaration("xml version=\"1.0\"");
  doc.InsertFirstChild(decl);

  tinyxml2::XMLElement *root = doc.NewElement("EtherCATConfig");
  root->SetAttribute("Version", version_.c_str());
  doc.InsertEndChild(root);

  tinyxml2::XMLElement *config = doc.NewElement("Config");
  root->InsertEndChild(config);

  // Master
  tinyxml2::XMLElement *master = doc.NewElement("Master");
  config->InsertEndChild(master);

  tinyxml2::XMLElement *info = doc.NewElement("Info");
  master->InsertEndChild(info);

  tinyxml2::XMLElement *mname = doc.NewElement("Name");
  mname->SetText(master_.name.c_str());
  info->InsertEndChild(mname);

  tinyxml2::XMLElement *mdst = doc.NewElement("Destination");
  mdst->SetText(master_.destination_mac.c_str());
  info->InsertEndChild(mdst);

  tinyxml2::XMLElement *msrc = doc.NewElement("Source");
  msrc->SetText(master_.source_mac.c_str());
  info->InsertEndChild(msrc);

  // Slaves
  for (const EniSlave &slave : slaves_) {
    tinyxml2::XMLElement *se = doc.NewElement("Slave");
    config->InsertEndChild(se);

    tinyxml2::XMLElement *si = doc.NewElement("Info");
    se->InsertEndChild(si);

    tinyxml2::XMLElement *sname = doc.NewElement("Name");
    sname->SetText(slave.name.c_str());
    si->InsertEndChild(sname);

    tinyxml2::XMLElement *pa = doc.NewElement("PhysAddr");
    pa->SetText(slave.phys_addr);
    si->InsertEndChild(pa);

    tinyxml2::XMLElement *vi = doc.NewElement("VendorId");
    vi->SetText(slave.vendor_id);
    si->InsertEndChild(vi);

    tinyxml2::XMLElement *pc = doc.NewElement("ProductCode");
    pc->SetText(slave.product_code);
    si->InsertEndChild(pc);

    tinyxml2::XMLElement *rn = doc.NewElement("RevisionNo");
    rn->SetText(slave.revision_no);
    si->InsertEndChild(rn);

    if (slave.previous_port.has_value()) {
      tinyxml2::XMLElement *ppe = doc.NewElement("PreviousPort");
      if (slave.previous_port->selected) {
        ppe->SetAttribute("Selected", "true");
      }
      tinyxml2::XMLElement *prt = doc.NewElement("Port");
      char pbuf[2] = {slave.previous_port->port, '\0'};
      prt->SetText(pbuf);
      ppe->InsertEndChild(prt);

      tinyxml2::XMLElement *ppa = doc.NewElement("PhysAddr");
      ppa->SetText(slave.previous_port->phys_addr);
      ppe->InsertEndChild(ppa);
      se->InsertEndChild(ppe);
    }
  }

  // Cyclic
  tinyxml2::XMLElement *cyclic = doc.NewElement("Cyclic");
  config->InsertEndChild(cyclic);

  tinyxml2::XMLElement *ct = doc.NewElement("CycleTime");
  ct->SetText(cycle_time_us_);
  cyclic->InsertEndChild(ct);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  return std::string(printer.CStr());
}

const EniSlave *EniDocument::find_slave_by_phys_addr(std::uint16_t phys_addr) const {
  for (const EniSlave &slave : slaves_) {
    if (slave.phys_addr == phys_addr) {
      return &slave;
    }
  }
  return nullptr;
}

const EniSlave *EniDocument::find_slave_by_alias(std::uint16_t alias) const {
  for (const EniSlave &slave : slaves_) {
    if (slave.hot_connect.has_value()) {
      for (const EniInitCmd &cmd : slave.hot_connect->identify_cmds) {
        if (cmd.validate.has_value() && cmd.validate->data.size() >= 2) {
          std::uint16_t val_alias = static_cast<std::uint16_t>(cmd.validate->data[0] | (cmd.validate->data[1] << 8));
          if (val_alias == alias) {
            return &slave;
          }
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<quasar::named::NamedObject>
EniDocument::to_named_tree(std::shared_ptr<quasar::named::NamedObject> parent) const {
  std::shared_ptr<quasar::named::NamedObject> root =
      quasar::named::NamedObject::create("EniConfig", parent);

  // Master node
  std::shared_ptr<quasar::named::NamedObject> master_node =
      quasar::named::NamedObject::create("Master", root);
  quasar::named::NamedString::create("Name", master_.name, master_node);
  quasar::named::NamedString::create("Destination", master_.destination_mac, master_node);
  quasar::named::NamedString::create("Source", master_.source_mac, master_node);

  // Slaves node
  std::shared_ptr<quasar::named::NamedObject> slaves_node =
      quasar::named::NamedObject::create("Slaves", root);
  for (const EniSlave &slave : slaves_) {
    std::string sname = slave.name.empty() ? ("Slave_" + std::to_string(slave.phys_addr)) : slave.name;
    std::shared_ptr<quasar::named::NamedObject> sn =
        quasar::named::NamedObject::create(sname, slaves_node);

    quasar::named::NamedInteger<std::uint32_t>::create("PhysAddr", slave.phys_addr, sn);
    quasar::named::NamedInteger<std::uint32_t>::create("VendorId", slave.vendor_id, sn);
    quasar::named::NamedInteger<std::uint32_t>::create("ProductCode", slave.product_code, sn);
    quasar::named::NamedInteger<std::uint32_t>::create("RevisionNo", slave.revision_no, sn);

    if (slave.previous_port.has_value()) {
      std::shared_ptr<quasar::named::NamedObject> pp_node =
          quasar::named::NamedObject::create("PreviousPort", sn);
      std::string pstr(1, slave.previous_port->port);
      quasar::named::NamedString::create("Port", pstr, pp_node);
      quasar::named::NamedInteger<std::uint32_t>::create("ParentPhysAddr", slave.previous_port->phys_addr, pp_node);
    }

    if (slave.hot_connect.has_value()) {
      std::shared_ptr<quasar::named::NamedObject> hc_node =
          quasar::named::NamedObject::create("HotConnect", sn);
      quasar::named::NamedString::create("GroupName", slave.hot_connect->group_name, hc_node);
      quasar::named::NamedInteger<std::uint32_t>::create("GroupMemberCount", slave.hot_connect->group_member_count, hc_node);
    }
  }

  return root;
}

} // namespace resoem::eni
