#pragma once

#include "resoem/EtherCATTypes.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace resoem {

struct SyncManagerInfo {
  uint16_t start_addr;
  uint16_t length;
  uint32_t flags;
  uint8_t type; // 1=MbxOut, 2=MbxIn, 3=Outputs, 4=Inputs
};

struct SlaveInfo {
  uint16_t configured_address;
  uint16_t alias_address;

  uint32_t vendor_id;
  uint32_t product_code;
  uint32_t revision_number;
  uint32_t serial_number;

  std::string name;

  uint8_t ports_link_status; // From DL Status
  int parent_index = -1;

  // Mailbox configuration
  uint16_t mbx_out_offset;
  uint16_t mbx_out_length;
  uint16_t mbx_in_offset;
  uint16_t mbx_in_length;
  uint16_t mbx_protocols;
  uint8_t mbx_cnt = 0; // Current counter [0..7]

  // Capabilities
  bool has_dc = false;
  uint8_t coe_details = 0;

  std::vector<SyncManagerInfo> sync_managers;
};

} // namespace resoem
