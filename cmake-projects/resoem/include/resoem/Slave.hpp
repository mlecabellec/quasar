/**
 * @file Slave.hpp
 * @brief EtherCAT Slave information and configuration structures.
 */

#pragma once

#include "resoem/EtherCATTypes.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace resoem {

/**
 * @brief Information about a SyncManager.
 */
struct SyncManagerInfo {
  uint16_t start_addr; ///< Physical start address in ESC
  uint16_t length;     ///< Length of the SyncManager area
  uint32_t flags;      ///< Control/Status flags
  uint8_t type;        ///< Type: 1=MbxOut, 2=MbxIn, 3=Outputs, 4=Inputs
};

/**
 * @brief Information about a Fieldbus Memory Management Unit (FMMU).
 */
struct FMMUInfo {
  uint32_t logical_start;     ///< Logical start address
  uint16_t length;            ///< Length in bytes
  uint8_t logical_start_bit;  ///< Start bit in logical byte
  uint8_t logical_end_bit;    ///< End bit in logical byte
  uint16_t physical_start;    ///< Physical start address in ESC
  uint8_t physical_start_bit; ///< Start bit in physical byte
  uint8_t type;               ///< Type: 1=Read (from phys), 2=Write (to phys)
  uint8_t active;             ///< Whether this FMMU is active
  uint8_t reserved1 = 0;      ///< Reserved
  uint16_t reserved2 = 0;     ///< Reserved
} __attribute__((packed));

/**
 * @brief Information about a PDO entry.
 */
struct PDOEntryInfo {
  uint16_t index;     ///< Object index
  uint8_t subindex;   ///< Object sub-index
  uint8_t bit_length; ///< Length in bits
  std::string name;   ///< Entry name (from SII or CoE)
};

/**
 * @brief Information about a Process Data Object (PDO).
 */
struct PDOInfo {
  uint16_t index;       ///< Object index
  uint8_t sync_manager; ///< SyncManager index this PDO is assigned to
  std::string name;     ///< PDO name
  std::vector<PDOEntryInfo> entries; ///< List of entries in this PDO
};

/**
 * @brief Comprehensive information about an EtherCAT Slave.
 */
struct SlaveInfo {
  uint16_t configured_address; ///< Configured Station Address
  uint16_t alias_address;      ///< Configured Station Alias

  uint32_t vendor_id;       ///< Vendor ID
  uint32_t product_code;    ///< Product Code
  uint32_t revision_number; ///< Revision Number
  uint32_t serial_number;   ///< Serial Number

  std::string name; ///< Device Name

  uint8_t ports_link_status;  ///< Link status of ESC ports
  int parent_index = -1;      ///< Index of the parent slave in the chain
  bool online = false;        ///< Whether the slave is currently online
  uint16_t current_state = 0; ///< Current AL State

  // Mailbox configuration
  uint16_t mbx_out_offset; ///< Mailbox Output (Master->Slave) offset
  uint16_t mbx_out_length; ///< Mailbox Output length
  uint16_t mbx_in_offset;  ///< Mailbox Input (Master->Slave) offset
  uint16_t mbx_in_length;  ///< Mailbox Input length
  uint16_t mbx_protocols;  ///< Supported mailbox protocols
  uint8_t mbx_cnt = 0;     ///< Current mailbox counter [0..7]

  // Topology
  struct PortInfo {
    bool active = false;      ///< Link is up
    bool loop_closed = false; ///< Loop closed at this port
    int neighbor_idx = -1;    ///< Index of connected slave (-1 if none/unknown)
  };
  std::array<PortInfo, 4> ports;     ///< Status of ports 0-3
  uint8_t parent_port = 0;           ///< Port on this slave connected to parent
  uint8_t entry_port = 0;            ///< Port where the frame arrived
  std::vector<int> children_indices; ///< Indices of child slaves

  // Capabilities
  bool has_dc = false;            ///< Supports Distributed Clocks
  uint32_t propagation_delay = 0; ///< DC propagation delay
  uint8_t coe_details = 0;        ///< CoE details (e.g. SDO info support)

  std::vector<SyncManagerInfo> sync_managers; ///< Configured SyncManagers
  std::vector<FMMUInfo> fmmu;                 ///< Configured FMMUs
  std::vector<PDOInfo> rx_pdos;               ///< Outputs (Master -> Slave)
  std::vector<PDOInfo> tx_pdos;               ///< Inputs (Slave -> Master)

  // Process data offsets in logical image
  uint32_t inputs_offset = 0;     ///< Offset of inputs in process image
  uint32_t inputs_size_bits = 0;  ///< Total size of inputs in bits
  uint32_t outputs_offset = 0;    ///< Offset of outputs in process image
  uint32_t outputs_size_bits = 0; ///< Total size of outputs in bits
};

} // namespace resoem
