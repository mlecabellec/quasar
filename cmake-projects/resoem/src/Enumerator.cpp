#include "resoem/Enumerator.hpp"
#include "resoem/Diagnostics.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/ProcessImage.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

namespace resoem {

Enumerator::Enumerator(RawSocket &socket) : socket_(socket) {}

Result<size_t> Enumerator::enumerate() {
  std::cout << "Starting enumeration..." << std::endl;
  slaves_.clear();
  reset_to_init();

  int slave_count = broadcast_read_count();
  std::cout << "Found " << slave_count << " slaves." << std::endl;
  if (slave_count <= 0) return 0;

  assign_addresses(slave_count);
  read_sii_data(slave_count);

  std::cout << "Transitioning all slaves to PRE_OP..." << std::endl;
  if (auto res = request_state_all(states::PRE_OP); !res) return std::unexpected(res.error());

  return static_cast<size_t>(slave_count);
}

Result<uint16_t> Enumerator::request_state(uint16_t slave_idx, uint16_t state,
                                   std::chrono::microseconds timeout) {
  if (slave_idx >= slaves_.size()) return std::unexpected(ECError::ProtocolError);
  uint16_t cfg_addr = slaves_[slave_idx].configured_address;
  write_register_fpwr<uint16_t>(cfg_addr, regs::AL_CONTROL, state);

  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout) {
    int wkc;
    uint16_t status = read_register_fprd<uint16_t>(cfg_addr, regs::AL_STATUS, wkc);
    if (wkc > 0) {
      uint16_t cur = status & al_status::STATE_MASK;
      if (cur == state) return cur;
      if (status & al_status::ERROR_BIT) {
        uint16_t code = read_register_fprd<uint16_t>(cfg_addr, regs::AL_STATUS_CODE, wkc);
        std::cerr << "Slave " << slave_idx + 1 << " AL Status Error: 0x" << std::hex << code << ": " << al_status_code_to_string(code) << std::dec << std::endl;
        write_register_fpwr<uint16_t>(cfg_addr, regs::AL_CONTROL, state | states::ACK);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return std::unexpected(ECError::Timeout);
}

Result<> Enumerator::request_state_all(uint16_t state, std::chrono::microseconds timeout) {
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, state);
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout) {
    bool all = true;
    for (size_t i = 0; i < slaves_.size(); ++i) {
      int wkc;
      uint16_t status = read_register_fprd<uint16_t>(slaves_[i].configured_address, regs::AL_STATUS, wkc);
      if (wkc > 0 && (status & al_status::STATE_MASK) != state) {
        all = false;
        if (status & al_status::ERROR_BIT) request_state(static_cast<uint16_t>(i), state, std::chrono::milliseconds(100));
      } else if (wkc <= 0) all = false;
    }
    if (all) return {};
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return std::unexpected(ECError::Timeout);
}

Result<uint32_t> Enumerator::configure_fmmu(ProcessImage &image) {
  uint32_t offset = 0;
  for (size_t i = 0; i < slaves_.size(); ++i) {
    SlaveInfo &info = slaves_[i];
    uint32_t out_bits = 0;
    for (const auto &pdo : info.rx_pdos) for (const auto &e : pdo.entries) out_bits += e.bit_length;
    if (out_bits > 0) {
      uint32_t bytes = (out_bits + 7) / 8;
      info.outputs_offset = offset; info.outputs_size_bits = out_bits;
      FMMUInfo fmmu{offset, static_cast<uint16_t>(bytes), 0, static_cast<uint8_t>((out_bits - 1) % 8), 0x1000, 0, 1, 1};
      for(const auto& sm : info.sync_managers) if(sm.type == 3) { fmmu.physical_start = sm.start_addr; break; }
      write_register_fpwr<FMMUInfo>(info.configured_address, regs::FMMU0, fmmu);
      info.fmmu.push_back(fmmu);
      offset += bytes;
    }
  }
  for (size_t i = 0; i < slaves_.size(); ++i) {
    SlaveInfo &info = slaves_[i];
    uint32_t in_bits = 0;
    for (const auto &pdo : info.tx_pdos) for (const auto &e : pdo.entries) in_bits += e.bit_length;
    if (in_bits > 0) {
      uint32_t bytes = (in_bits + 7) / 8;
      info.inputs_offset = offset; info.inputs_size_bits = in_bits;
      FMMUInfo fmmu{offset, static_cast<uint16_t>(bytes), 0, static_cast<uint8_t>((in_bits - 1) % 8), 0x1100, 0, 2, 1};
      for(const auto& sm : info.sync_managers) if(sm.type == 4) { fmmu.physical_start = sm.start_addr; break; }
      uint16_t reg = info.fmmu.size() > 0 ? regs::FMMU1 : regs::FMMU0;
      write_register_fpwr<FMMUInfo>(info.configured_address, reg, fmmu);
      info.fmmu.push_back(fmmu);
      offset += bytes;
    }
  }
  image.resize(offset);
  return offset;
}

Result<uint16_t> Enumerator::exchange_process_data(ProcessImage &image, std::chrono::microseconds timeout) {
  if (image.size() == 0) return 0;
  std::span<byte> data = image.data();
  int wkc = send_receive(cmds::LRW, 0, 0, data);
  if (wkc < 0) return std::unexpected(ECError::Timeout);
  return static_cast<uint16_t>(wkc);
}

void Enumerator::measure_propagation_delays() {
  for (auto &s : slaves_) {
    int wkc; uint8_t f = read_register_fprd<uint8_t>(s.configured_address, regs::ESC_FEATURES, wkc);
    s.has_dc = (wkc > 0) && (f & 0x04);
    if (s.has_dc) { write_register_fpwr<uint64_t>(s.configured_address, regs::DC_SYS_TIME_OFFSET, 0); write_register_fpwr<uint32_t>(s.configured_address, regs::DC_SYS_TIME_DELAY, 0); }
  }
  for (int i = 0; i < 10; ++i) { uint32_t z = 0; send_receive(cmds::BWR, 0, regs::DC_RECEIVE_TIME_PORT0, std::span<byte>(reinterpret_cast<byte*>(&z), 4)); }
  int ref = -1; for (size_t i = 0; i < slaves_.size(); ++i) if (slaves_[i].has_dc) { ref = static_cast<int>(i); break; }
  if (ref == -1) return;
  slaves_[ref].propagation_delay = 0;
  for (size_t i = ref + 1; i < slaves_.size(); ++i) {
     if (!slaves_[i].has_dc) continue;
     int wkc; uint32_t p1 = read_register_fprd<uint32_t>(slaves_[i-1].configured_address, regs::DC_RECEIVE_TIME_PORT1, wkc);
     uint32_t c0 = read_register_fprd<uint32_t>(slaves_[i].configured_address, regs::DC_RECEIVE_TIME_PORT0, wkc);
     slaves_[i].propagation_delay = slaves_[i-1].propagation_delay + (c0 > p1 ? (c0 - p1) : 300);
     write_register_fpwr<uint32_t>(slaves_[i].configured_address, regs::DC_SYS_TIME_DELAY, slaves_[i].propagation_delay);
  }
}

void Enumerator::sync_clocks() {
  int ref = -1; for (size_t i = 0; i < slaves_.size(); ++i) if (slaves_[i].has_dc) { ref = static_cast<int>(i); break; }
  if (ref == -1) return;
  uint64_t t = 0; send_receive(cmds::ARMW, static_cast<uint16_t>(-ref), regs::DC_SYS_TIME, std::span<byte>(reinterpret_cast<byte*>(&t), 8));
}

void Enumerator::configure_dc(SlaveInfo &s, uint32_t cyc, int32_t shift) {
  if (!s.has_dc) return;
  write_register_fpwr<uint8_t>(s.configured_address, regs::DC_SYNC_ACT, 0);
  write_register_fpwr<uint32_t>(s.configured_address, regs::DC_SYNC0_CYCLE_TIME, cyc);
  int wkc; uint64_t cur = read_register_fprd<uint64_t>(s.configured_address, regs::DC_SYS_TIME, wkc);
  uint64_t start = ((cur + 100'000'000) / cyc) * cyc + shift;
  write_register_fpwr<uint64_t>(s.configured_address, regs::DC_SYNC_START_TIME, start);
  write_register_fpwr<uint8_t>(s.configured_address, regs::DC_SYNC_ACT, 0x03);
}

void Enumerator::check_slaves_status() {
  for (auto &s : slaves_) {
    int wkc; uint16_t st = read_register_fprd<uint16_t>(s.configured_address, regs::AL_STATUS, wkc);
    if (wkc > 0) { s.online = true; s.current_state = st & al_status::STATE_MASK; } else s.online = false;
  }
}

bool Enumerator::recover_slave(int idx) {
  if (idx >= (int)slaves_.size()) return false;
  write_register_apwr<uint16_t>(static_cast<uint16_t>(-idx), regs::CONFIG_STATION_ADDR, slaves_[idx].configured_address);
  int wkc; read_register_fprd<uint16_t>(slaves_[idx].configured_address, regs::AL_STATUS, wkc);
  if (wkc > 0) { slaves_[idx].online = true; return true; }
  return false;
}

void Enumerator::reset_to_init() {
  write_register_broadcast<uint8_t>(regs::DL_ALIAS, 0);
  uint16_t al_ctl = states::INIT | states::ACK;
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);
  write_register_broadcast<uint8_t>(regs::DL_PORT, 0);
  write_register_broadcast<uint16_t>(regs::IRQ_MASK, 0x0004);
  std::array<byte, 8> zero_buf{};
  send_receive(cmds::BWR, 0x0000, regs::RX_ERR, zero_buf);
  std::vector<byte> large_zero(128, static_cast<byte>(0));
  send_receive(cmds::BWR, 0x0000, regs::FMMU0, large_zero);
  send_receive(cmds::BWR, 0x0000, regs::SM0, large_zero);
  write_register_broadcast<uint8_t>(regs::DC_SYNC_ACT, 0);
  send_receive(cmds::BWR, 0x0000, regs::DC_SYS_TIME, std::span<byte>(large_zero.data(), 4));
  write_register_broadcast<uint16_t>(regs::DC_SPEED_CNT, 0x1000);
  write_register_broadcast<uint16_t>(regs::DC_TIME_FILT, 0x0C00);
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 2);
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 0);
}

int Enumerator::broadcast_read_count() {
  int wkc = 0;
  read_register_broadcast<uint8_t>(regs::TYPE, wkc);
  return wkc;
}

void Enumerator::assign_addresses(int count) {
  for (int i = 1; i <= count; ++i) {
    slaves_.push_back({});
    SlaveInfo &info = slaves_.back();
    uint16_t auto_inc_addr = static_cast<uint16_t>(1 - i);
    uint16_t config_addr = 0x1000 + i;
    write_register_apwr<uint16_t>(auto_inc_addr, regs::CONFIG_STATION_ADDR, config_addr);
    info.configured_address = config_addr;
    info.online = true;
    if (i == 1) write_register_fpwr<uint16_t>(config_addr, regs::DL_CONTROL, 0x0000);
    int wkc;
    uint16_t dl_status = read_register_fprd<uint16_t>(config_addr, regs::DL_STATUS, wkc);
    if (wkc > 0) info.ports_link_status = (dl_status >> 8) & 0x0F;
  }
}

int Enumerator::send_receive(uint8_t cmd, uint16_t addr, uint16_t offset, std::span<byte> data) {
  FrameBuilder builder; uint8_t idx = current_idx_++;
  builder.add_datagram(cmd, idx, addr, offset, data);
  auto frame = builder.build();
  socket_.send(frame);
  std::vector<byte> rx_buffer(1500);
  size_t received = socket_.receive(rx_buffer);
  if (received == 0) return -1;
  size_t wkc_offset = 14 + 2 + 10 + data.size();
  if (received < wkc_offset + 2) return -2;
  uint16_t wkc; std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);
  if (wkc > 0 || (cmd & 0x1)) std::memcpy(data.data(), rx_buffer.data() + 14 + 2 + 10, data.size());
  return wkc;
}

template <typename T> T Enumerator::read_register_broadcast(uint16_t reg, int &wkc) {
  T val{}; std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  wkc = send_receive(cmds::BRD, 0x0000, reg, buf); return val;
}
template <typename T> int Enumerator::write_register_broadcast(uint16_t reg, const T &value) {
  T temp = value; std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::BWR, 0x0000, reg, buf);
}
template <typename T> int Enumerator::write_register_apwr(uint16_t auto_inc_addr, uint16_t reg, const T &value) {
  T temp = value; std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::APWR, auto_inc_addr, reg, buf);
}
template <typename T> T Enumerator::read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc) {
  T val{}; std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  wkc = send_receive(cmds::FPRD, configured_addr, reg, buf); return val;
}
template <typename T> int Enumerator::write_register_fpwr(uint16_t configured_addr, uint16_t reg, const T &value) {
  T temp = value; std::span<byte> buf(reinterpret_cast<byte *>(&temp), sizeof(T));
  return send_receive(cmds::FPWR, configured_addr, reg, buf);
}

uint32_t Enumerator::read_sii_word(uint16_t slave_cfg_addr, uint16_t word_addr) {
  int wkc; uint16_t eep_stat = read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);
  bool use8 = (wkc > 0) && (eep_stat & regs::ESTAT_R64);
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_ADDRESS, word_addr);
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, eeprom::CMD_READ);
  for (int i = 0; i < 100; ++i) {
    uint16_t status = read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);
    if (wkc > 0 && !(status & eeprom::BUSY)) {
      if (status & eeprom::ERROR_MASK) return 0xFFFFFFFF;
      return read_register_fprd<uint32_t>(slave_cfg_addr, regs::EEPROM_DATA, wkc);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  return 0xFFFFFFFF;
}

uint16_t Enumerator::find_sii_category(uint16_t slave_cfg_addr, uint16_t cat_type) {
  uint16_t ptr = 0x0040;
  while (ptr < 0xFFFF) {
    uint32_t res = read_sii_word(slave_cfg_addr, ptr);
    if (res == 0xFFFFFFFF) break;
    uint16_t type = static_cast<uint16_t>(res & 0xFFFF);
    if (type == 0xFFFF) break;
    uint16_t size = static_cast<uint16_t>(res >> 16);
    if (type == cat_type) return ptr + 1;
    ptr += size + 1;
  }
  return 0;
}

std::string Enumerator::read_sii_string(uint16_t slave_cfg_addr, uint8_t string_idx) {
  if (string_idx == 0) return "";
  uint16_t cat_ptr = find_sii_category(slave_cfg_addr, 0x000A);
  if (cat_ptr == 0) return "";
  uint32_t res = read_sii_word(slave_cfg_addr, cat_ptr);
  uint8_t num_strings = static_cast<uint8_t>(res & 0xFF);
  if (string_idx > num_strings) return "";
  uint16_t current_ptr = cat_ptr; uint8_t current_offset = 1;
  for (uint8_t i = 1; i <= string_idx; ++i) {
    uint16_t word = static_cast<uint16_t>(read_sii_word(slave_cfg_addr, current_ptr) >> (8 * (current_offset % 2)));
    uint8_t len = static_cast<uint8_t>(word & 0xFF);
    if (i == string_idx) {
      std::string s;
      for (uint8_t j = 0; j < len; ++j) {
        uint8_t byte_offset = (current_offset + 1 + j);
        uint32_t word_val = read_sii_word(slave_cfg_addr, current_ptr + (byte_offset / 2));
        s += static_cast<char>((word_val >> (8 * (byte_offset % 2))) & 0xFF);
      }
      return s;
    }
    current_offset += len + 1; current_ptr += current_offset / 2; current_offset %= 2;
  }
  return "";
}

void Enumerator::read_sii_categories(int slave_idx) {
  SlaveInfo &info = slaves_[slave_idx]; uint16_t addr = info.configured_address;
  info.vendor_id = read_sii_word(addr, 0x0008); info.product_code = read_sii_word(addr, 0x000A);
  info.name = read_sii_string(addr, 1);
  uint16_t gen_ptr = find_sii_category(addr, 0x001E);
  if (gen_ptr > 0) info.coe_details = static_cast<uint8_t>((read_sii_word(addr, gen_ptr + 3) >> 8) & 0xFF);
  uint16_t sm_ptr = find_sii_category(addr, 0x0029);
  if (sm_ptr > 0) {
    for (int i = 0; i < 16; ++i) {
      uint32_t w12 = read_sii_word(addr, sm_ptr + i * 4);
      uint32_t w34 = read_sii_word(addr, sm_ptr + i * 4 + 2);
      if (w12 == 0xFFFFFFFF) break;
      SyncManagerInfo sm; sm.start_addr = static_cast<uint16_t>(w12 & 0xFFFF); sm.length = static_cast<uint16_t>(w12 >> 16); sm.flags = w34;
      uint8_t ctrl = static_cast<uint8_t>(sm.flags & 0xFF);
      bool is_mailbox = ((ctrl >> 2) & 0x03) == 0x02;
      bool is_out = (ctrl & 0x01) == 0;
      if (is_mailbox) {
        if (is_out) { info.mbx_out_offset = sm.start_addr; info.mbx_out_length = sm.length; sm.type = 1; }
        else { info.mbx_in_offset = sm.start_addr; info.mbx_in_length = sm.length; sm.type = 2; }
      }
      if (sm.length == 0) break;
      info.sync_managers.push_back(sm);
    }
  }
}

void Enumerator::read_sii_pdos(int slave_idx) {
  SlaveInfo &info = slaves_[slave_idx]; uint16_t addr = info.configured_address;
  auto parse = [&](uint16_t cat, std::vector<PDOInfo> &pdos) {
    uint16_t ptr = find_sii_category(addr, cat); if (ptr == 0) return;
    uint32_t sz = (read_sii_word(addr, ptr - 1) >> 16) * 2;
    uint16_t cur = 0;
    while (cur < sz) {
      PDOInfo pdo; uint32_t w1 = read_sii_word(addr, ptr + (cur / 2));
      pdo.index = static_cast<uint16_t>(w1 & 0xFFFF); uint8_t num = static_cast<uint8_t>((w1 >> 16) & 0xFF);
      pdo.sync_manager = static_cast<uint8_t>((w1 >> 24) & 0xFF);
      cur += 8;
      for (uint8_t i = 0; i < num; ++i) {
        PDOEntryInfo e; uint32_t ew1 = read_sii_word(addr, ptr + (cur / 2)); uint32_t ew2 = read_sii_word(addr, ptr + (cur / 2) + 2);
        e.index = static_cast<uint16_t>(ew1 & 0xFFFF); e.subindex = static_cast<uint8_t>((ew1 >> 16) & 0xFF);
        e.bit_length = static_cast<uint8_t>((ew2 >> 8) & 0xFF); pdo.entries.push_back(e); cur += 8;
      }
      pdos.push_back(pdo);
    }
  };
  parse(eeprom::CAT_PDO_RX, info.rx_pdos); parse(eeprom::CAT_PDO_TX, info.tx_pdos);
}

void Enumerator::map_topology(int count) {
  for (int i = 0; i < count; ++i) {
    slaves_[i].parent_index = (i == 0) ? -1 : i - 1;
  }
}

void Enumerator::read_sii_data(int count) {
  for (int i = 0; i < count; ++i) { read_sii_categories(i); read_sii_pdos(i); }
  map_topology(count);
}

} // namespace resoem
