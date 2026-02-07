#include "resoem/Enumerator.hpp"
#include "resoem/EtherCATFrame.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

namespace resoem {

Enumerator::Enumerator(RawSocket &socket) : socket_(socket) {}

int Enumerator::enumerate() {
  std::cout << "Starting enumeration..." << std::endl;
  slaves_.clear();

  // 1. Reset everything
  reset_to_init();

  // 2. Count slaves
  int slave_count = broadcast_read_count();
  std::cout << "Found " << slave_count << " slaves." << std::endl;

  if (slave_count <= 0) {
    return 0;
  }

  // 3. Assign addresses
  assign_addresses(slave_count);

  // 4. Read SII (EEPROM) info
  read_sii_data(slave_count);

  return slave_count;
}

void Enumerator::reset_to_init() {
  // 1. NetX100 Workaround & AL Alias Reset
  write_register_broadcast<uint8_t>(regs::DL_ALIAS, 0);

  // Write INIT state to AL Control (Broadcast) - First reset
  uint16_t al_ctl = states::INIT | states::ACK;
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);

  // Second reset to satisfy NetX100
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);

  // 2. Default Register Setup (from SOEM ecx_set_slaves_to_default)
  // Deactivate loop manual
  write_register_broadcast<uint8_t>(regs::DL_PORT, 0);

  // Set IRQ mask
  write_register_broadcast<uint16_t>(regs::IRQ_MASK, 0x0004); // SOEM default

  // Reset CRC counters (8 bytes at 0x0300)
  std::array<byte, 8> zero_buf{};
  send_receive(cmds::BWR, 0x0000, regs::RX_ERR, zero_buf);

  // Reset FMMUs (base 0x0600) and SyncManagers (base 0x0800)
  // SOEM resets 16*3 FMMUs and 8*4 SyncManagers. Let's do a large zero write.
  std::vector<byte> large_zero(128, static_cast<byte>(0));
  send_receive(cmds::BWR, 0x0000, regs::FMMU0, large_zero);
  send_receive(cmds::BWR, 0x0000, regs::SM0, large_zero);

  // Reset DC registers
  write_register_broadcast<uint8_t>(regs::DC_SYNC_ACT, 0);
  send_receive(cmds::BWR, 0x0000, regs::DC_SYS_TIME,
               std::span<byte>(large_zero.data(), 4));
  write_register_broadcast<uint16_t>(regs::DC_SPEED_CNT,
                                     0x1000); // DC speedstart
  write_register_broadcast<uint16_t>(regs::DC_TIME_FILT,
                                     0x0C00); // DC filt expr

  // Force EEPROM from PDI and then to Master
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 2); // Force PDI
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 0); // Set to master
}

int Enumerator::broadcast_read_count() {
  // Broadcast Reading the TYPE register (0x0000)
  // The working counter returned will be equal to the number of slaves
  // processing the frame.
  int wkc = 0;
  read_register_broadcast<uint8_t>(regs::TYPE, wkc);
  return wkc;
}

void Enumerator::assign_addresses(int count) {
  // For each slave 1..count
  // Auto Increment Address:
  // Master sends 0.
  // Slave positions are 0, 1, 2...
  // Wait, Standard Auto Inc:
  // Master sends to 0. First slave receives 0. Increments to 1.
  // Wait, the SOEM code used `1 - slave`.
  // If slave 1: 1-1 = 0.
  // If slave 2: 1-2 = -1 (0xFFFF).
  //
  // Let's verify how EtherCAT Auto-Inc works.
  // The datagram has an ADP.
  // Each slave increments ADP.
  // If the slave sees ADP == 0, it processes the datagram.
  // So if we want to address the 1st slave (first on wire), we send ADP=0.
  // If we want the 2nd slave, we send ADP = -1 (0xFFFF). First slave increments
  // to 0. Second slave sees 0. If we want 3rd slave, ADP = -2 (0xFFFE).

  // SOEM code: `ADPh = (uint16)(1 - slave)`. Correct.

  for (int i = 1; i <= count; ++i) {
    slaves_.push_back({});
    SlaveInfo &info = slaves_.back();

    uint16_t auto_inc_addr = static_cast<uint16_t>(1 - i);
    uint16_t config_addr =
        0x1000 +
        i; // Start at 0x1001 like SOEM? SOEM uses slave + NODEOFFSET(0x1000).

    // Read PDI Control to check interface (optional, for debug)
    // int wkc;
    // read_register_aprd<uint16_t>(auto_inc_addr, regs::PDI_CONTROL, wkc);

    // Write Configured Station Address
    write_register_apwr<uint16_t>(auto_inc_addr, regs::CONFIG_STATION_ADDR,
                                  config_addr);
    info.configured_address = config_addr;

    // Coverage: kill non-EtherCAT frames on first slave (SOEM behavior)
    if (i == 1) {
      write_register_fpwr<uint16_t>(config_addr, regs::DL_CONTROL, 0x0000);
    }

    // Read DL Status to capture link status
    int wkc;
    uint16_t dl_status =
        read_register_fprd<uint16_t>(config_addr, regs::DL_STATUS, wkc);
    if (wkc > 0) {
      info.ports_link_status = (dl_status >> 8) & 0x0F;
    }

    std::cout << "Slave " << i << " assigned address 0x" << std::hex
              << config_addr << " (Links: 0x" << (int)info.ports_link_status
              << ")" << std::dec << std::endl;

    // Use FPRD to read ALIAS to verify address setup
    // uint16_t alias = read_register_fprd<uint16_t>(config_addr,
    // regs::CONFIG_STATION_ALIAS, wkc);
  }
}

// ... send_receive implementation ...
int Enumerator::send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                             std::span<byte> data) {
  FrameBuilder builder;
  uint8_t idx = current_idx_++;

  builder.add_datagram(cmd, idx, addr, offset, data);
  auto frame = builder.build();

  socket_.send(frame);

  std::vector<byte> rx_buffer(1500);
  size_t received = socket_.receive(rx_buffer);
  if (received == 0)
    return -1; // Timeout

  // Find WKC (Assuming single datagram for now)
  size_t wkc_offset = 14 + 2 + 10 + data.size();
  if (received < wkc_offset + 2)
    return -2;

  uint16_t wkc;
  std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);

  // Copy data back if WKC > 0 (or for any read)
  if (wkc > 0 ||
      (cmd &
       0x1)) { // Read commands are odd in SOEM? No, APRD=1, FPRD=4, BRD=7.
    // Actually APRD=1, FPRD=4, BRD=7, LRD=10. All these are reads.
    // Commands are: APRD=1, APWR=2, APRW=3, FPRD=4, FPWR=5, FPRW=6, BRD=7,
    // BWR=8, BRW=9... So Reads are 1, 4, 7, 10. Odd? 1 is odd, 4 is even, 7 is
    // odd, 10 is even. Let's just copy back if WKC > 0.
    std::memcpy(data.data(), rx_buffer.data() + 14 + 2 + 10, data.size());
  }

  return wkc;
}

// Template implementations
template <typename T>
T Enumerator::read_register_broadcast(uint16_t reg, int &wkc) {
  T val{};
  std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  wkc = send_receive(cmds::BRD, 0x0000, reg, buf);
  return val;
}

template <typename T>
int Enumerator::write_register_broadcast(uint16_t reg, const T &value) {
  T temp = value;
  std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::BWR, 0x0000, reg, buf);
}

template <typename T>
int Enumerator::write_register_apwr(uint16_t auto_inc_addr, uint16_t reg,
                                    const T &value) {
  T temp = value;
  std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::APWR, auto_inc_addr, reg, buf);
}

template <typename T>
T Enumerator::read_register_fprd(uint16_t configured_addr, uint16_t reg,
                                 int &wkc) {
  T val{};
  std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  wkc = send_receive(cmds::FPRD, configured_addr, reg, buf);
  return val;
}

template <typename T>
int Enumerator::write_register_fpwr(uint16_t configured_addr, uint16_t reg,
                                    const T &value) {
  T temp = value;
  std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::FPWR, configured_addr, reg, buf);
}

uint32_t Enumerator::read_sii_word(uint16_t slave_cfg_addr,
                                   uint16_t word_addr) {
  // 1. Check if slave supports 8-byte read (optional, we could cache this)
  int wkc;
  uint16_t eep_stat =
      read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);
  bool use8 = (wkc > 0) && (eep_stat & regs::ESTAT_R64);

  // 2. Write Address
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_ADDRESS,
                                word_addr);

  // 3. Command Read (0x0100)
  uint16_t cmd = eeprom::CMD_READ;
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, cmd);

  // 4. Poll
  for (int i = 0; i < 100; ++i) {
    uint16_t status =
        read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);

    if (wkc > 0 && !(status & eeprom::BUSY)) {
      if (status & eeprom::ERROR_MASK)
        return 0xFFFFFFFF;

      // 5. Read Data
      if (use8) {
        // If 8 bytes, we still return 32 bits for now as requested by SII
        // Categories But we could return uint64_t. SII Categories usually need
        // 32 or 16.
        uint32_t data = read_register_fprd<uint32_t>(slave_cfg_addr,
                                                     regs::EEPROM_DATA, wkc);
        return data;
      } else {
        uint32_t data = read_register_fprd<uint32_t>(slave_cfg_addr,
                                                     regs::EEPROM_DATA, wkc);
        return data;
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  return 0xFFFFFFFF;
}

uint16_t Enumerator::find_sii_category(uint16_t slave_cfg_addr,
                                       uint16_t cat_type) {
  uint16_t ptr = 0x0040; // Categories start after header
  while (ptr < 0xFFFF) {
    uint32_t res = read_sii_word(slave_cfg_addr, ptr);
    if (res == 0xFFFFFFFF)
      break;

    uint16_t type = static_cast<uint16_t>(res & 0xFFFF);
    if (type == 0xFFFF)
      break; // End of categories

    uint16_t size = static_cast<uint16_t>(res >> 16);
    if (type == cat_type)
      return ptr + 1; // Return start of data

    ptr += size + 1; // Move to next category header
  }
  return 0;
}

std::string Enumerator::read_sii_string(uint16_t slave_cfg_addr,
                                        uint8_t string_idx) {
  if (string_idx == 0)
    return "";
  uint16_t cat_ptr =
      find_sii_category(slave_cfg_addr, 0x000A); // Strings category
  if (cat_ptr == 0)
    return "";

  uint32_t res = read_sii_word(slave_cfg_addr, cat_ptr);
  uint8_t num_strings = static_cast<uint8_t>(res & 0xFF);
  if (string_idx > num_strings)
    return "";

  // Traverse strings
  uint16_t current_ptr = cat_ptr;
  uint8_t current_offset = 1; // Skip num_strings

  for (uint8_t i = 1; i <= string_idx; ++i) {
    // Read length
    uint16_t word =
        static_cast<uint16_t>(read_sii_word(slave_cfg_addr, current_ptr) >>
                              (8 * (current_offset % 2)));
    uint8_t len = static_cast<uint8_t>(word & 0xFF);

    if (i == string_idx) {
      std::string s;
      for (uint8_t j = 0; j < len; ++j) {
        uint8_t byte_offset = (current_offset + 1 + j);
        uint32_t word_val =
            read_sii_word(slave_cfg_addr, current_ptr + (byte_offset / 2));
        s += static_cast<char>((word_val >> (8 * (byte_offset % 2))) & 0xFF);
      }
      return s;
    }

    current_offset += len + 1;
    current_ptr += current_offset / 2;
    current_offset %= 2;
  }
  return "";
}

void Enumerator::read_sii_categories(int slave_idx) {
  SlaveInfo &info = slaves_[slave_idx];
  uint16_t addr = info.configured_address;

  // 1. Mandatory Header Identity
  uint32_t id1 = read_sii_word(addr, 0x0008);
  uint32_t id2 = read_sii_word(addr, 0x000A);
  info.vendor_id = id1;
  info.product_code = id2;

  // 2. Name from Strings (Index 1 is usually the name)
  info.name = read_sii_string(addr, 1);

  // 3. General Category
  uint16_t gen_ptr =
      find_sii_category(addr, 0x001E); // General category is 30 (0x001E)
  if (gen_ptr > 0) {
    uint32_t w1 = read_sii_word(addr, gen_ptr + 3); // Words 3-4?
    // SOEM: CoEdetails = ecx_siigetbyte(context, slave, ssigen + 0x07);
    // Categorized word addressing in find_sii_category returns ptr to data.
    // ssigen + 0x07 (byte) -> word (gen_ptr + 3) byte 1
    uint32_t w4 = read_sii_word(addr, gen_ptr + 3);
    info.coe_details = static_cast<uint8_t>((w4 >> 8) & 0xFF);
  }

  // 4. SyncManagers
  uint16_t sm_ptr = find_sii_category(addr, 0x0029); // SM category
  if (sm_ptr > 0) {
    // Each SM is 4 words
    for (int i = 0; i < 16; ++i) { // Max 16 SMs
      uint32_t w12 = read_sii_word(addr, sm_ptr + i * 4);
      uint32_t w34 = read_sii_word(addr, sm_ptr + i * 4 + 2);
      if (w12 == 0xFFFFFFFF)
        break;

      SyncManagerInfo sm;
      sm.start_addr = static_cast<uint16_t>(w12 & 0xFFFF);
      sm.length = static_cast<uint16_t>(w12 >> 16);
      sm.flags = w34;

      // SM Type from flags: bits 16-18 of w34?
      // SOEM uses (Creg) + (Activate << 16)
      // Standard:
      // SM Control Register (flags & 0xFFFF)
      // bit 2-3: Operation Mode (0: 3-buffer, 2: Mailbox)
      // bit 0-1: SM Type (0: Read/Master2Slave, 1: Write/Slave2Master)

      uint8_t ctrl = static_cast<uint8_t>(sm.flags & 0xFF);
      bool is_mailbox = ((ctrl >> 2) & 0x03) == 0x02;
      bool is_out = (ctrl & 0x01) == 0; // 0 = write to slave (out)

      if (is_mailbox) {
        if (is_out) {
          info.mbx_out_offset = sm.start_addr;
          info.mbx_out_length = sm.length;
          sm.type = 1; // MbxOut
        } else {
          info.mbx_in_offset = sm.start_addr;
          info.mbx_in_length = sm.length;
          sm.type = 2; // MbxIn
        }
      }

      if (sm.length == 0)
        break;
      info.sync_managers.push_back(sm);
    }
  }
}

void Enumerator::map_topology(int count) {
  // Basic Tracing:
  // In a simple chain, slave (i) is parent of (i+1).
  // In more complex topologies, we need to look at open ports.

  for (int i = 0; i < count; ++i) {
    if (i == 0) {
      slaves_[i].parent_index = -1; // Master is parent
    } else {
      // Heuristic: The parent is the last slave that has an open downstream
      // port. In a simple chain, this is always i-1. If we have junctions
      // (Hubs/Branches), this gets more complex. For Phase 2, we will stick to
      // the chain assumption but log the port status.
      slaves_[i].parent_index = i - 1;
    }

    // Count active ports for logging
    int active_ports = 0;
    for (int p = 0; p < 4; ++p) {
      if (slaves_[i].ports_link_status & (1 << p))
        active_ports++;
    }
    std::cout << "Slave " << (i + 1) << " (" << slaves_[i].name
              << "): Parent=" << slaves_[i].parent_index
              << " ActivePorts=" << active_ports << " (Mask: 0x" << std::hex
              << (int)slaves_[i].ports_link_status << std::dec << ")"
              << std::endl;
  }
}

void Enumerator::read_sii_data(int count) {
  for (int i = 0; i < count; ++i) {
    std::cout << "Reading SII for slave " << (i + 1) << "..." << std::endl;
    read_sii_categories(i);
    std::cout << "  Name: " << slaves_[i].name << " (Vendor: 0x" << std::hex
              << slaves_[i].vendor_id << std::dec << ")" << std::endl;
  }
  map_topology(count);
}

} // namespace resoem
