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

struct FMMUInfo {
  uint32_t logical_start;
  uint16_t length;
  uint8_t logical_start_bit;
  uint8_t logical_end_bit;
  uint16_t physical_start;
  uint8_t physical_start_bit;
  uint8_t type; // 1=Read (from phys), 2=Write (to phys)
  uint8_t active;
  uint8_t reserved1 = 0;
  uint16_t reserved2 = 0;
} __attribute__((packed));

struct PDOEntryInfo {
  uint16_t index;
  uint8_t subindex;
  uint8_t bit_length;
  std::string name;
};

struct PDOInfo {
  uint16_t index;
  uint8_t sync_manager;
  std::string name;
  std::vector<PDOEntryInfo> entries;
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
  bool online = false;
  uint16_t current_state = 0;

  // Mailbox configuration
  uint16_t mbx_out_offset;
  uint16_t mbx_out_length;
  uint16_t mbx_in_offset;
  uint16_t mbx_in_length;
  uint16_t mbx_protocols;
  uint8_t mbx_cnt = 0; // Current counter [0..7]

  // Capabilities
  bool has_dc = false;
  uint32_t propagation_delay = 0;
  uint8_t coe_details = 0;

  std::vector<SyncManagerInfo> sync_managers;
  std::vector<FMMUInfo> fmmu;
  std::vector<PDOInfo> rx_pdos; // Outputs (Master -> Slave)
  std::vector<PDOInfo> tx_pdos; // Inputs (Slave -> Master)

  // Process data offsets in logical image
  uint32_t inputs_offset = 0;
  uint32_t inputs_size_bits = 0;
  uint32_t outputs_offset = 0;
  uint32_t outputs_size_bits = 0;
};

} // namespace resoem
