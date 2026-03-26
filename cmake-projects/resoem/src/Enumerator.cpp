#include "resoem/Enumerator.hpp"
#include "resoem/CoEHandler.hpp"
#include "resoem/Diagnostics.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/ProcessImage.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <numeric>
#include <thread>

namespace resoem {

Enumerator::Enumerator(RawSocket &socket) : socket_(socket) {}

Result<size_t> Enumerator::enumerate() {
  if (verbose_) {
    std::cout << "[VERBOSE] Starting network enumeration..." << std::endl;
  }
  slaves_.clear();

  // Reset the network to a known state (INIT, clear FMMUs/SMs, etc.)
  if (verbose_) {
    std::cout << "[VERBOSE] Resetting slaves to INIT and clearing ESC registers..." << std::endl;
  }
  reset_to_init();

  // [FE-0040.3.1] Automatically detect and count slaves on the wire using broadcast read.
  int slave_count = broadcast_read_count();
  if (verbose_) {
    std::cout << "[VERBOSE] Broadcast read (BRD) found " << slave_count << " slaves on the bus." << std::endl;
  } else {
    std::cout << "Found " << slave_count << " slaves." << std::endl;
  }
  
  if (slave_count <= 0)
    return 0;

  // [FE-0040.3.2] Assign configured station addresses (starting at 0x1001) to each slave.
  if (verbose_) {
    std::cout << "[VERBOSE] Assigning station addresses starting at 0x1001..." << std::endl;
  }
  assign_addresses(slave_count);

  // [FE-0040.3.3] Parse information (Vendor, Product, PDOs) from the SII (EEPROM) of each slave.
  if (verbose_) {
    std::cout << "[VERBOSE] Reading SII (EEPROM) data for all slaves..." << std::endl;
  }
  read_sii_data(slave_count);

  // Configure Mailbox SyncManagers (type 1 = Out, type 2 = In) before PRE_OP
  if (verbose_) {
    std::cout << "[VERBOSE] Configuring SyncManagers for Mailbox communication..." << std::endl;
  }
  struct SMConfig {
    uint16_t start_addr;
    uint16_t length;
    uint32_t flags;
  } __attribute__((packed));

  for (size_t s_idx = 0; s_idx < slaves_.size(); ++s_idx) {
    SlaveInfo &info = slaves_[s_idx];
    if (verbose_) {
      std::cout << "[VERBOSE] Slave " << s_idx << " (" << info.name << "):" << std::endl;
    }
    for (size_t i = 0; i < info.sync_managers.size(); ++i) {
      const SyncManagerInfo &sm = info.sync_managers[i];
      if (sm.type == 1 || sm.type == 2) {
        // Ensure Enable bit is set (Activate register, bit 0 of the second 16-bit word)
        // SII Category 0x0029 Word 3 Bit 0 is the Enable bit.
        // In our 32-bit flags: [Control(8) | Status(8) | Activate(8) | PDI(8)]
        // Activate is flags >> 16. Bit 16 is Activate bit 0.
        uint32_t final_flags = sm.flags | 0x00010000;
        SMConfig cfg{sm.start_addr, sm.length, final_flags};
        
        if (verbose_) {
          std::cout << "  - SM" << i << " (Mbx " << (sm.type == 1 ? "Out" : "In") 
                    << "): Start=0x" << std::hex << sm.start_addr 
                    << ", Len=" << std::dec << sm.length 
                    << ", Flags=0x" << std::hex << final_flags << std::dec << std::endl;
        }

        uint16_t sm_reg = regs::SM0 + static_cast<uint16_t>(i * 8);
        write_register_fpwr<SMConfig>(info.configured_address, sm_reg, cfg);
      }
    }
  }

  // Request all slaves to move to the PRE-OP state to allow mailbox
  // communication.
  if (verbose_) {
    std::cout << "[VERBOSE] Requesting transition to PRE_OP for all slaves..." << std::endl;
  }
  Result<> res = request_state_all(states::PRE_OP);
  if (!res) {
    if (verbose_) {
      std::cerr << "[VERBOSE] Failed to reach PRE_OP state: " << res.error() << std::endl;
    }
    return std::unexpected(res.error());
  }

  return static_cast<size_t>(slave_count);
}

Result<uint16_t> Enumerator::request_state(uint16_t slave_idx, uint16_t state,
                                           std::chrono::microseconds timeout) {
  if (slave_idx >= slaves_.size())
    return std::unexpected(ECError::ProtocolError);

  uint16_t cfg_addr = slaves_[slave_idx].configured_address;
  if (verbose_) {
    std::cout << "[VERBOSE] Slave " << slave_idx << ": Requesting state 0x" 
              << std::hex << state << std::dec << "..." << std::endl;
  }

  // Write requested state to the AL Control register.
  write_register_fpwr<uint16_t>(cfg_addr, regs::AL_CONTROL, state);

  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < 1000000; ++i) {
    if (std::chrono::steady_clock::now() - start >= timeout)
      break;
    int wkc;
    // Read current state from the AL Status register.
    uint16_t status =
        read_register_fprd<uint16_t>(cfg_addr, regs::AL_STATUS, wkc);
    if (wkc > 0) {
      uint16_t cur = status & regs::al_status::STATE_MASK;
      if (cur == (state & regs::al_status::STATE_MASK)) {
        if (verbose_) {
          std::cout << "[VERBOSE] Slave " << slave_idx << ": Reached state 0x" 
                    << std::hex << cur << std::dec << "." << std::endl;
        }
        return cur;
      }

      // If the error bit is set, read the error code and try to acknowledge it.
      if (status & regs::al_status::ERROR_BIT) {
        uint16_t code =
            read_register_fprd<uint16_t>(cfg_addr, regs::AL_STATUS_CODE, wkc);
        std::cerr << "Slave " << slave_idx << " AL Status Error: 0x"
                  << std::hex << code << ": " << al_status_code_to_string(code)
                  << std::dec << std::endl;
        
        if (verbose_) {
          std::cout << "[VERBOSE] Slave " << slave_idx << ": Acknowledging error 0x" 
                    << std::hex << code << std::dec << "..." << std::endl;
        }
        // Write the error acknowledge bit.
        write_register_fpwr<uint16_t>(cfg_addr, regs::AL_CONTROL,
                                      (state & regs::al_status::STATE_MASK) | states::ACK);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (i == 999999)
      throw std::runtime_error("Hard limit exceeded in request_state");
  }
  return std::unexpected(ECError::Timeout);
}

Result<> Enumerator::request_state_all(uint16_t state,
                                       std::chrono::microseconds timeout) {
  if (verbose_) {
    std::cout << "[VERBOSE] Requesting state 0x" << std::hex << state 
              << std::dec << " for all slaves via broadcast..." << std::endl;
  }
  // Use a broadcast write to request state for all slaves at once.
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, state);

  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < 1000000; ++i) {
    if (std::chrono::steady_clock::now() - start >= timeout)
      break;
    bool all = true;
    for (size_t j = 0; j < slaves_.size(); ++j) {
      int wkc;
      uint16_t status = read_register_fprd<uint16_t>(
          slaves_[j].configured_address, regs::AL_STATUS, wkc);

      // Check if this slave has reached the target state.
      if (wkc > 0) {
        uint16_t cur = status & regs::al_status::STATE_MASK;
        if (cur != (state & regs::al_status::STATE_MASK)) {
          all = false;
          // If there is an error, try to handle it for this specific slave.
          if (status & regs::al_status::ERROR_BIT) {
            uint16_t code = read_register_fprd<uint16_t>(
                slaves_[j].configured_address, regs::AL_STATUS_CODE, wkc);
            std::cerr << "Slave " << j << " AL Status Error: 0x" << std::hex
                      << code << ": " << al_status_code_to_string(code)
                      << std::dec << std::endl;
            // Write the error acknowledge bit.
            write_register_fpwr<uint16_t>(slaves_[j].configured_address,
                                          regs::AL_CONTROL,
                                          (state & regs::al_status::STATE_MASK) | states::ACK);
          }
        }
      } else {
        all = false;
      }
    }

    if (all) {
      if (verbose_) {
        std::cout << "[VERBOSE] All slaves reached state 0x" << std::hex 
                  << state << std::dec << "." << std::endl;
      }
      return {};
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (i == 999999)
      throw std::runtime_error("Hard limit exceeded in request_state_all");
  }
  return std::unexpected(ECError::Timeout);
}

Result<uint32_t> Enumerator::configure_fmmu(ProcessImage &image) {
  uint32_t offset = 0;

  // [FE-0040.5.2] Automatically calculate and program FMMU (Fieldbus Memory Management Unit) entries for inputs and outputs.
  // First pass: Configure FMMUs for Outputs (Master -> Slave).
  for (size_t i = 0; i < slaves_.size(); ++i) {
    SlaveInfo &info = slaves_[i];
    uint32_t out_bits = 0;
    out_bits = std::accumulate(
        info.rx_pdos.begin(), info.rx_pdos.end(), 0u,
        [](uint32_t sum, const PDOInfo &pdo) {
          return sum + std::accumulate(pdo.entries.begin(), pdo.entries.end(),
                                       0u,
                                       [](uint32_t s, const PDOEntryInfo &e) {
                                         return s + e.bit_length;
                                       });
        });

    if (out_bits > 0) {
      uint32_t bytes = (out_bits + 7) / 8;
      info.outputs_offset = offset;
      info.outputs_size_bits = out_bits;

      // Create FMMU configuration: map logical address 'offset' to physical
      // memory.
      FMMUInfo fmmu{offset, static_cast<uint16_t>(bytes),
                    0,      static_cast<uint8_t>((out_bits - 1) % 8),
                    0x1000, 0,
                    1,  // Type: Read (from physical)
                    1}; // Active

      // Find the appropriate SyncManager for outputs.
      std::vector<SyncManagerInfo>::const_iterator sm_it =
          std::find_if(info.sync_managers.begin(), info.sync_managers.end(),
                       [](const SyncManagerInfo &sm) { return sm.type == 3; });
      if (sm_it != info.sync_managers.end()) {
        fmmu.physical_start = sm_it->start_addr;
      }

      write_register_fpwr<FMMUInfo>(info.configured_address, regs::FMMU0, fmmu);
      info.fmmu.push_back(fmmu);
      offset += bytes;
    }
  }

  // Second pass: Configure FMMUs for Inputs (Slave -> Master).
  for (size_t i = 0; i < slaves_.size(); ++i) {
    SlaveInfo &info = slaves_[i];
    uint32_t in_bits = 0;
    in_bits = std::accumulate(
        info.tx_pdos.begin(), info.tx_pdos.end(), 0u,
        [](uint32_t sum, const PDOInfo &pdo) {
          return sum + std::accumulate(pdo.entries.begin(), pdo.entries.end(),
                                       0u,
                                       [](uint32_t s, const PDOEntryInfo &e) {
                                         return s + e.bit_length;
                                       });
        });

    if (in_bits > 0) {
      uint32_t bytes = (in_bits + 7) / 8;
      info.inputs_offset = offset;
      info.inputs_size_bits = in_bits;

      FMMUInfo fmmu{offset, static_cast<uint16_t>(bytes),
                    0,      static_cast<uint8_t>((in_bits - 1) % 8),
                    0x1100, 0,
                    2,  // Type: Write (to physical)
                    1}; // Active

      // Find the appropriate SyncManager for inputs.
      std::vector<SyncManagerInfo>::const_iterator sm_it =
          std::find_if(info.sync_managers.begin(), info.sync_managers.end(),
                       [](const SyncManagerInfo &sm) { return sm.type == 4; });
      if (sm_it != info.sync_managers.end()) {
        fmmu.physical_start = sm_it->start_addr;
      }

      // Use FMMU1 if FMMU0 is already taken by outputs.
      uint16_t reg = info.fmmu.size() > 0 ? regs::FMMU1 : regs::FMMU0;
      write_register_fpwr<FMMUInfo>(info.configured_address, reg, fmmu);
      info.fmmu.push_back(fmmu);
      offset += bytes;
    }
  }

  // [FE-0040.5.3] Manage a global ProcessImage buffer with bit-level accessors.
  // Resize the process image to hold all discovered data.
  image.resize(offset);
  return offset;
}

Result<uint16_t>
Enumerator::exchange_process_data(ProcessImage &image,
                                  std::chrono::microseconds timeout) {
  if (image.size() == 0)
    return 0;

  // [FE-0040.5.4] Support cyclic data exchange using the LRW command for atomic read/write updates.
  std::span<byte> data = image.data();
  // Use Logical ReadWrite (LRW) to exchange the entire process image in one
  // datagram.
  int wkc = send_receive(cmds::LRW, 0, 0, data);
  if (wkc < 0)
    return std::unexpected(ECError::Timeout);

  return static_cast<uint16_t>(wkc);
}

void Enumerator::sync_clocks() {
  // [FE-0040.6.2] designate a Reference Clock (usually the first DC-capable slave) for the network.
  // Find the reference clock.
  int ref = -1;
  for (size_t i = 0; i < slaves_.size(); ++i) {
    if (slaves_[i].has_dc) {
      ref = static_cast<int>(i);
      break;
    }
  }
  if (ref == -1)
    return;

  // [FE-0040.6.3] Provide cyclic drift compensation using the ARMW command to 0x0910.
  // Use ARMW to read from reference and write to all others.
  uint64_t t = 0;
  send_receive(cmds::ARMW, static_cast<uint16_t>(-ref), regs::DC_SYS_TIME,
               std::span<byte>(reinterpret_cast<byte *>(&t), 8));
}

void Enumerator::configure_dc(SlaveInfo &s, uint32_t cyc, int32_t shift) {
  if (!s.has_dc)
    return;

  // [FE-0040.6.4] Configure SYNC0/SYNC1 signals with specific cycle times and shift offsets.
  // 1. Deactivate sync.
  write_register_fpwr<uint8_t>(s.configured_address, regs::DC_SYNC_ACT, 0);
  // 2. Set cycle time.
  write_register_fpwr<uint32_t>(s.configured_address, regs::DC_SYNC0_CYCLE_TIME,
                                cyc);
  // 3. Set start time in the future.
  int wkc;
  uint64_t cur = read_register_fprd<uint64_t>(s.configured_address,
                                              regs::DC_SYS_TIME, wkc);
  uint64_t start = ((cur + 100'000'000) / cyc) * cyc + shift;
  write_register_fpwr<uint64_t>(s.configured_address, regs::DC_SYNC_START_TIME,
                                start);
  // 4. Activate SYNC0 and SYNC1.
  write_register_fpwr<uint8_t>(s.configured_address, regs::DC_SYNC_ACT, 0x03);
}

void Enumerator::check_slaves_status() {
  for (SlaveInfo &s : slaves_) {
    int wkc;
    uint16_t st = read_register_fprd<uint16_t>(s.configured_address,
                                               regs::AL_STATUS, wkc);
    if (wkc > 0) {
      s.online = true;
      s.current_state = st & regs::al_status::STATE_MASK;
    } else {
      s.online = false;
    }
  }
}

bool Enumerator::recover_slave(int idx) {
  if (idx >= (int)slaves_.size())
    return false;

  // [FE-0040.7.1] Implement "Hot-Connect" support via automated slave recovery.
  // Try to re-assign the configured station address using auto-increment.
  write_register_apwr<uint16_t>(static_cast<uint16_t>(-idx),
                                regs::CONFIG_STATION_ADDR,
                                slaves_[idx].configured_address);

  // Verify if the slave responds to its configured address.
  int wkc;
  read_register_fprd<uint16_t>(slaves_[idx].configured_address, regs::AL_STATUS,
                               wkc);
  if (wkc > 0) {
    slaves_[idx].online = true;
    return true;
  }
  return false;
}

void Enumerator::reset_to_init() {
  // Clear any existing aliases and request INIT state with error
  // acknowledgment.
  write_register_broadcast<uint8_t>(regs::DL_ALIAS, 0);
  uint16_t al_ctl = states::INIT | states::ACK;
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, al_ctl);

  // Clear DL ports, IRQ mask, and error counters.
  write_register_broadcast<uint8_t>(regs::DL_PORT, 0);
  write_register_broadcast<uint16_t>(regs::IRQ_MASK, 0x0004);
  std::array<byte, 8> zero_buf{};
  send_receive(cmds::BWR, 0x0000, regs::RX_ERR, zero_buf);

  // Clear FMMUs and SyncManagers.
  std::vector<byte> large_zero(128, static_cast<byte>(0));
  send_receive(cmds::BWR, 0x0000, regs::FMMU0, large_zero);
  send_receive(cmds::BWR, 0x0000, regs::SM0, large_zero);

  // Reset Distributed Clocks settings.
  write_register_broadcast<uint8_t>(regs::DC_SYNC_ACT, 0);
  send_receive(cmds::BWR, 0x0000, regs::DC_SYS_TIME,
               std::span<byte>(large_zero.data(), 4));
  write_register_broadcast<uint16_t>(regs::DC_SPEED_CNT, 0x1000);
  write_register_broadcast<uint16_t>(regs::DC_TIME_FILT, 0x0C00);

  // Toggle EEPROM configuration to reset SII interface.
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 2);
  write_register_broadcast<uint8_t>(regs::REG_EEPCFG, 0);
}

int Enumerator::broadcast_read_count() {
  int wkc = 0;
  // BRD to register 0 (ESC Type) increments WKC for every slave that sees it.
  read_register_broadcast<uint8_t>(regs::TYPE, wkc);
  return wkc;
}

void Enumerator::assign_addresses(int count) {
  for (int i = 1; i <= count; ++i) {
    slaves_.push_back({});
    SlaveInfo &info = slaves_.back();
    // Auto-increment address for the i-th slave in the chain.
    uint16_t auto_inc_addr = static_cast<uint16_t>(1 - i);
    uint16_t config_addr = 0x1000 + i;

    // Assign the new Station Address.
    write_register_apwr<uint16_t>(auto_inc_addr, regs::CONFIG_STATION_ADDR,
                                  config_addr);
    info.configured_address = config_addr;
    info.online = true;

    // For the first slave, ensure DL control is initialized.
    if (i == 1)
      write_register_fpwr<uint16_t>(config_addr, regs::DL_CONTROL, 0x0000);

    int wkc;
    uint16_t dl_status =
        read_register_fprd<uint16_t>(config_addr, regs::DL_STATUS, wkc);
    if (wkc > 0)
      info.ports_link_status = (dl_status >> 8) & 0x0F;
  }
}

int Enumerator::send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                             std::span<byte> data) {
  FrameBuilder builder;
  uint8_t idx = current_idx_++;
  builder.add_datagram(cmd, idx, addr, offset, data);
  std::span<const byte> frame = builder.build();

  // Send the frame.
  // In a full redundancy scenario, we might want to send on both if the loop is
  // broken. For now, we rely on the primary send, and if the cable is
  // redundant, it returns on secondary.
  try {
    socket_.send(frame);
  } catch (const SocketError &) {
    return -1;
  }

  // Wait for the response with matching index.
  std::vector<byte> rx_buffer(1500);
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();

  while (true) {
    static uint64_t loop_count = 0;
    if (++loop_count > 10'000'000)
      throw std::runtime_error("Hard limit exceeded in send_receive loop");
    int port_idx = 0;
    size_t received = socket_.receive(rx_buffer, &port_idx);

    if (received == 0) {
      // Check timeout
      std::chrono::steady_clock::time_point now =
          std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
              .count() > 100) {
        return -1; // Timeout
      }
      continue;
    }

    // Basic size check: Eth(14) + ECat(2) + DgramHeader(10) + WKC(2) = 28
    if (received < 28)
      continue;

    // Check EtherType (0x88A4) - RawSocket binds to it, but good to be sure if
    // we used ETH_P_ALL Check Index matches Eth(14) + ECat(2) -> Datagram
    // starts at 16. Cmd(1) at 16, Idx(1) at 17.
    uint8_t received_idx = static_cast<uint8_t>(rx_buffer[17]);

    if (received_idx == idx) {
      // Process this frame.
      // Note: If we receive on port_idx == 1 (Secondary), it means redundancy
      // is working!

      size_t wkc_offset = 16 + 10 + data.size(); // 14+2 = 16
      if (received < wkc_offset + 2)
        return -2; // Fragmented

      uint16_t wkc;
      std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);

      if (wkc > 0 || (cmd & 0x1) || (cmd == cmds::BWR)) // Read or WKC>0
        std::memcpy(data.data(), rx_buffer.data() + 16 + 10, data.size());

      return wkc;
    }
    // If index doesn't match, loop again (ignore other frames/delayed frames)
  }
}

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
  int wkc;
  (void)read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);

  // Set address to read from.
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_ADDRESS,
                                word_addr);
  // Issue the read command.
  write_register_fpwr<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL,
                                eeprom::CMD_READ);

  // Poll for the BUSY bit to clear.
  for (int i = 0; i < 100; ++i) {
    uint16_t status =
        read_register_fprd<uint16_t>(slave_cfg_addr, regs::EEPROM_CONTROL, wkc);
    if (wkc > 0 && !(status & eeprom::BUSY)) {
      if (status & eeprom::ERROR_MASK)
        return 0xFFFFFFFF;
      // Read the data from the EEPROM data register.
      return read_register_fprd<uint32_t>(slave_cfg_addr, regs::EEPROM_DATA,
                                          wkc);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  return 0xFFFFFFFF;
}

uint16_t Enumerator::find_sii_category(uint16_t slave_cfg_addr,
                                       uint16_t cat_type) {
  // SII categories start at word 0x0040.
  uint16_t ptr = 0x0040;
  for (uint32_t loop_guard = 0; ptr < 0xFFFF && loop_guard < 10000;
       ++loop_guard) {
    uint32_t res = read_sii_word(slave_cfg_addr, ptr);
    if (res == 0xFFFFFFFF)
      break;
    uint16_t type = static_cast<uint16_t>(res & 0xFFFF);
    if (type == 0xFFFF)
      break; // End of categories
    uint16_t size = static_cast<uint16_t>(res >> 16);
    if (type == cat_type)
      return ptr + 2; // Return pointer to category data
    ptr += size + 2;  // Skip to next category
    if (loop_guard == 9999)
      throw std::runtime_error("Hard limit reached in find_sii_category");
  }
  return 0;
}

std::string Enumerator::read_sii_string(uint16_t slave_cfg_addr,
                                        uint8_t string_idx) {
  if (string_idx == 0)
    return "";
  uint16_t cat_ptr = find_sii_category(slave_cfg_addr, 0x000A);
  if (cat_ptr == 0)
    return "";

  uint32_t res = read_sii_word(slave_cfg_addr, cat_ptr);
  uint8_t num_strings = static_cast<uint8_t>(res & 0xFF);
  if (string_idx > num_strings)
    return "";

  uint16_t current_ptr = cat_ptr;
  uint8_t current_offset = 1;
  for (uint8_t i = 1; i <= string_idx; ++i) {
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

  // Read basic device information.
  info.vendor_id = read_sii_word(addr, 0x0008);
  info.product_code = read_sii_word(addr, 0x000A);
  info.name = read_sii_string(addr, 1);

  // Read CoE details from General category.
  uint16_t gen_ptr = find_sii_category(addr, 0x001E);
  if (gen_ptr > 0)
    info.coe_details =
        static_cast<uint8_t>((read_sii_word(addr, gen_ptr + 3) >> 8) & 0xFF);

  // Read SyncManager configurations.
  uint16_t sm_ptr = find_sii_category(addr, 0x0029);
  if (sm_ptr > 0) {
    for (int i = 0; i < 16; ++i) {
      uint32_t w12 = read_sii_word(addr, sm_ptr + i * 4);
      uint32_t w34 = read_sii_word(addr, sm_ptr + i * 4 + 2);
      if (w12 == 0xFFFFFFFF)
        break;

      SyncManagerInfo sm;
      sm.start_addr = static_cast<uint16_t>(w12 & 0xFFFF);
      sm.length = static_cast<uint16_t>(w12 >> 16);
      sm.flags = w34;

      // ESC SyncManager Control Byte (Register 0x0804):
      // Bits 0-1: Operating Mode (00=Buffered, 10=Mailbox)
      // Bits 2-3: Direction (00=Read/Master Read, 01=Write/Master Write)
      uint8_t ctrl = static_cast<uint8_t>(sm.flags & 0xFF);
      bool is_mailbox = (ctrl & 0x03) == 0x02;
      bool is_write = ((ctrl >> 2) & 0x03) == 0x01; // Master -> Slave

      if (is_mailbox) {
        if (is_write) {
          info.mbx_out_offset = sm.start_addr;
          info.mbx_out_length = sm.length;
          sm.type = 1; // Mailbox Out (Master -> Slave)
        } else {
          info.mbx_in_offset = sm.start_addr;
          info.mbx_in_length = sm.length;
          sm.type = 2; // Mailbox In (Slave -> Master)
        }
      }
      if (sm.length == 0)
        break;
      info.sync_managers.push_back(sm);
    }
  }
}

void Enumerator::read_sii_pdos(int slave_idx) {
  SlaveInfo &info = slaves_[slave_idx];
  uint16_t addr = info.configured_address;

  // Lambda to parse RxPDO or TxPDO categories.
  std::function<void(uint16_t, std::vector<PDOInfo> &)> parse =
      [&](uint16_t cat, std::vector<PDOInfo> &pdos) {
        uint16_t ptr = find_sii_category(addr, cat);
        if (ptr == 0)
          return;
        uint32_t sz = (read_sii_word(addr, ptr - 2) >> 16) * 2;
        uint16_t cur = 0;
        for (uint32_t loop_guard = 0; cur < sz && loop_guard < 10000;
             ++loop_guard) {
          PDOInfo pdo;
          uint32_t w1 = read_sii_word(addr, ptr + (cur / 2));
          pdo.index = static_cast<uint16_t>(w1 & 0xFFFF);
          uint8_t num = static_cast<uint8_t>((w1 >> 16) & 0xFF);
          pdo.sync_manager = static_cast<uint8_t>((w1 >> 24) & 0xFF);
          cur += 8;
          for (uint8_t i = 0; i < num; ++i) {
            PDOEntryInfo e;
            uint32_t ew1 = read_sii_word(addr, ptr + (cur / 2));
            uint32_t ew2 = read_sii_word(addr, ptr + (cur / 2) + 2);
            e.index = static_cast<uint16_t>(ew1 & 0xFFFF);
            e.subindex = static_cast<uint8_t>((ew1 >> 16) & 0xFF);
            e.bit_length = static_cast<uint8_t>((ew2 >> 8) & 0xFF);
            pdo.entries.push_back(e);
            cur += 8;
          }
          pdos.push_back(pdo);
          if (loop_guard == 9999)
            throw std::runtime_error(
                "Hard limit reached in read_sii_pdos loop");
        }
      };

  parse(eeprom::CAT_PDO_RX, info.rx_pdos);
  parse(eeprom::CAT_PDO_TX, info.tx_pdos);
  parse(eeprom::CAT_PDO_TX, info.tx_pdos);
}

void Enumerator::read_sii_data(int count) {
  for (int i = 0; i < count; ++i) {
    read_sii_categories(i);
    read_sii_pdos(i);
  }
  map_topology(count);
}

void Enumerator::read_port_status() {
  for (SlaveInfo &slave : slaves_) {
    int wkc;
    // DL Status (0x0110)
    // Bits 0-3: Port 0 link (0=down, 1=up) ... Port 3 link
    // Bits 4-7: Loop closed 0..3
    // Bits 8-11: Signal detect 0..3
    uint16_t dl_status = read_register_fprd<uint16_t>(slave.configured_address,
                                                      regs::DL_STATUS, wkc);
    if (wkc <= 0)
      continue;

    slave.ports_link_status = dl_status & 0xFFFF; // Store raw value too?

    for (int p = 0; p < 4; ++p) {
      bool link = (dl_status & (1 << p)) != 0;
      bool loop = (dl_status & (1 << (p + 4))) != 0;
      slave.ports[p].active = link;
      slave.ports[p].loop_closed = loop;
    }
  }
}

void Enumerator::map_topology(int count) {
  // [FE-0040.3.4] Map network topology and establish parent-child relationships for each slave.
  read_port_status();

  if (slaves_.empty())
    return;

  // Reset topology info
  for (SlaveInfo &s : slaves_) {
    s.parent_index = -1;
    s.children_indices.clear();
  }

  // Slave 0 is the root (attached to Master)
  // We iterate through slaves 1..N and find their parent.
  // The parent is the first previous slave that has an open, active port.
  // "Open" means the port is active but not yet closed/consumed?
  // Actually, standard EtherCAT enumeration follows the wiring order.
  // 1. Packet goes into Port 0 (Entry).
  // 2. Internal processing.
  // 3. Fowarded to Port 3 (if active), then Port 1, then Port 2.
  // 4. Returns from 2 -> 1 -> 3 -> 0.
  //
  // So for slave[i], we trace back to find who sent it the packet.
  // We need to track which ports on previous slaves have been "consumed" by
  // slaves [1..i-1].

  // Initialize consumed ports tracking
  // We can use a temporary valid/available port bitmask for each slave.
  std::vector<uint8_t> available_ports(count);
  for (int i = 0; i < count; ++i) {
    // A port is available for a CHILD if it is active.
    // Port 0 is usually the entry port from PARENT, so it's not available for
    // children (except in redundancy, but let's assume Port 0 is entry).
    // Available output ports order: 3, 1, 2. (ESC logic).
    // But we just need to know which port connects to the *next* slave in the
    // auto-inc list.

    // Actually, the simpler SOEM logic:
    // "scan unconsumed ports in parent, consume and return first open port"
    // Order: 3 -> 1 -> 2 -> 0.

    uint8_t mask = 0;
    if (slaves_[i].ports[3].active)
      mask |= (1 << 3);
    if (slaves_[i].ports[1].active)
      mask |= (1 << 1);
    if (slaves_[i].ports[2].active)
      mask |= (1 << 2);
    if (slaves_[i].ports[0].active)
      mask |= (1 << 0);
    available_ports[i] = mask;
  }

  // Slave 0 is the root.
  // It consumes Port 0 (connected to Master).
  if (available_ports[0] & 1)
    available_ports[0] &= ~1;
  slaves_[0].entry_port = 0;

  for (int i = 1; i < count; ++i) {
    // Find parent for slave[i]
    // We look backwards from i-1.
    // The first slave we find with an available port (in order 3-1-2-0) is the
    // parent.
    int parent = -1;
    uint8_t pport = 0;

    for (int j = i - 1; j >= 0; --j) {
      uint8_t avail = available_ports[j];
      if (avail & (1 << 3)) {
        parent = j;
        pport = 3;
        break;
      }
      if (avail & (1 << 1)) {
        parent = j;
        pport = 1;
        break;
      }
      if (avail & (1 << 2)) {
        parent = j;
        pport = 2;
        break;
      }
      if (avail & (1 << 0)) {
        parent = j;
        pport = 0;
        break;
      }
    }

    if (parent != -1) {
      slaves_[i].parent_index = parent;
      slaves_[i].entry_port = 0; // Assumption: always enters at Port 0
      slaves_[i].parent_port = pport;

      slaves_[parent].children_indices.push_back(i);
      slaves_[parent].ports[pport].neighbor_idx = i;

      // Mark port as consumed on parent
      available_ports[parent] &= ~(1 << pport);

      // Mark entry port as consumed on child (Port 0)
      if (available_ports[i] & 1)
        available_ports[i] &= ~1;
    }
  }
}

uint32_t Enumerator::read_eeprom(uint16_t slave_idx, uint16_t word_addr) {
  if (slave_idx >= slaves_.size())
    return 0xFFFFFFFF;
  return read_sii_word(slaves_[slave_idx].configured_address, word_addr);
}

int Enumerator::write_eeprom(uint16_t slave_idx, uint16_t word_addr,
                             uint16_t data) {
  if (slave_idx >= slaves_.size())
    return 0;

  uint16_t config_addr = slaves_[slave_idx].configured_address;

  // 1. Force EEPROM control to Master (if it was PDI)
  // Check if we need to release PDI control first?
  // Logic from ecx_eeprom2master:
  // Write 2 to EEPCFG to force PDI, then 0 to set Master.
  // Here we just try to claim it.
  write_register_fpwr<uint8_t>(config_addr, regs::REG_EEPCFG, 2); // Force PDI
  write_register_fpwr<uint8_t>(config_addr, regs::REG_EEPCFG, 0); // Master

  // 2. Wait for not busy
  int wkc;
  for (int i = 0; i < 200; ++i) {
    uint16_t stat =
        read_register_fprd<uint16_t>(config_addr, regs::EEPROM_CONTROL, wkc);
    if (wkc > 0 && !(stat & eeprom::BUSY))
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 3. Write Data
  write_register_fpwr<uint16_t>(config_addr, regs::EEPROM_DATA, data);

  // 4. Issue Write Command
  // Command: Write (0x200) | Address
  // Actually Check regs::EEPROM_ADDRESS usage in read_sii_word.
  // In read_sii_word: write Addr to REG_EEPADR, then Cmd to REG_EEPCTL.
  // But ecx_writeeepromFP writes:
  // EEPDAT = data
  // EEPCTL = CMD_WRITE | Address? No.
  // SOEM struct ec_eepromt has comm, addr, d2.
  // The registers are:
  // 0x502 Control/Status (2 bytes)
  // 0x504 Address (4 bytes? No, usually 2 or 4).
  // 0x508 Data (4 or 8 bytes)
  //
  // SOEM ecx_writeeepromFP logic:
  // Write Data to 0x508.
  // Write {CMD_WRITE, Address} to 0x502?
  // Wait, ec_eepromt definition:
  // uint16 comm (0x502)
  // uint16 addr (0x504)
  // uint16 d2   (0x506?)
  //
  // It writes the WHOLE struct `ec_eepromt` to 0x502?
  // "wkc = ecx_FPWR(..., ECT_REG_EEPCTL, sizeof(ed), &ed, ...)"
  // sizeof(ed) = 6 bytes.
  // ECT_REG_EEPCTL is 0x502.
  // So it writes 0x502..0x507.
  // 0x502: Control (2 bytes)
  // 0x504: Address (2 bytes? or 4?)
  // SII Address is 32-bit in some, but usually 16-bit word address.
  //
  // Let's follow the standard:
  // Write Address to 0x504.
  // Write Data to 0x508.
  // Write Cmd to 0x502.

  write_register_fpwr<uint16_t>(config_addr, regs::EEPROM_ADDRESS, word_addr);
  write_register_fpwr<uint16_t>(config_addr, regs::EEPROM_CONTROL,
                                eeprom::CMD_WRITE);

  // 5. Wait for completion (Busy bit clear)
  for (int i = 0; i < 100; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    uint16_t stat =
        read_register_fprd<uint16_t>(config_addr, regs::EEPROM_CONTROL, wkc);
    if (wkc > 0 && !(stat & eeprom::BUSY)) {
      if (stat & eeprom::ERROR_MASK)
        return 0; // Error
      return 1;   // Success
    }
  }
  return 0; // Timeout
}

void Enumerator::load_eni(const ec_enit *eni) {
  if (!eni)
    return;

  std::cout << "Loading ENI configuration..." << std::endl;
  MailboxHandler mbx(socket_);
  CoEHandler coe_handler(mbx);

  for (int i = 0; i < eni->slavecount; ++i) {
    const ec_enislavet &es = eni->slave[i];

    // Match slave by index (assuming 1:1 mapping for now)
    if (i >= static_cast<int>(slaves_.size())) {
      std::cerr << "ENI slave index " << i << " out of range." << std::endl;
      continue;
    }

    SlaveInfo &info = slaves_[i];
    // Optional: Verify Identity
    if (info.vendor_id != es.VendorId) {
      std::cerr << "Warning: Slave " << i
                << " Vendor ID mismatch (ENI: " << std::hex << es.VendorId
                << ", Found: " << info.vendor_id << ")" << std::dec
                << std::endl;
    }

    std::cout << "Configuring Slave " << i << " (" << info.name << ")..."
              << std::endl;

    for (int j = 0; j < es.CoECmdCount; ++j) {
      const ec_enicoecmdt &cmd = es.CoECmds[j];

      // We apply commands regardless of transition for this test implementation
      std::span<const uint8_t> data(static_cast<const uint8_t *>(cmd.Data),
                                    cmd.DataSize);

      Result<> res =
          coe_handler.sdo_write(info, cmd.Index, cmd.SubIdx, data, cmd.CA != 0);
      if (!res) {
        std::cerr << "Failed to write SDO 0x" << std::hex << cmd.Index << ":"
                  << (int)cmd.SubIdx << std::dec << " to slave " << i
                  << std::endl;
      }
    }
  }
}

// Helper to find the previous active port in the processing order 0 -> 3 -> 1
// -> 2
static uint8_t get_prev_port(uint8_t current_port,
                             const std::array<SlaveInfo::PortInfo, 4> &ports) {
  // Order: 0 -> 3 -> 1 -> 2 -> 0
  // Reverse: 2 <- 1 <- 3 <- 0

  if (current_port == 0) {
    if (ports[2].active)
      return 2;
    if (ports[1].active)
      return 1;
    if (ports[3].active)
      return 3;
    return 0; // Should not happen if 0 is active
  } else if (current_port == 1) {
    if (ports[3].active)
      return 3;
    if (ports[0].active)
      return 0;
    if (ports[2].active)
      return 2; // Loop back?
    return 1;
  } else if (current_port == 2) {
    if (ports[1].active)
      return 1;
    if (ports[3].active)
      return 3;
    if (ports[0].active)
      return 0;
    return 2;
  } else if (current_port == 3) {
    if (ports[0].active)
      return 0;
    if (ports[2].active)
      return 2;
    if (ports[1].active)
      return 1;
    return 3;
  }
  return 0;
}

void Enumerator::measure_propagation_delays() {
  if (slaves_.empty())
    return;

  // [FE-0040.6.1] Implement propagation delay measurement between slaves with nanosecond resolution.
  // 1. Latch receive times (BWR to 0x0900)
  uint32_t val = 0;
  write_register_broadcast<uint32_t>(regs::DC_RECEIVE_TIME_PORT0, 0);

  // Retrieve receive times for all slaves
  std::vector<std::array<uint32_t, 4>> recv_times(slaves_.size());

  for (size_t i = 0; i < slaves_.size(); ++i) {
    int wkc;
    // Read 0x0900..0x090F (4 x 32-bit)
    // We can read 16 bytes at once
    struct RecvTime {
      uint32_t t[4];
    };
    RecvTime rt = read_register_fprd<RecvTime>(
        slaves_[i].configured_address, regs::DC_RECEIVE_TIME_PORT0, wkc);
    if (wkc > 0) {
      recv_times[i][0] = rt.t[0]; // Port 0
      recv_times[i][1] = rt.t[1]; // Port 1
      recv_times[i][2] = rt.t[2]; // Port 2
      recv_times[i][3] = rt.t[3]; // Port 3
    }
  }

  // 2. Calculate delays
  slaves_[0].propagation_delay = 0;
  slaves_[0].has_dc = true; // Assuming root has DC for now

  for (size_t i = 1; i < slaves_.size(); ++i) {
    SlaveInfo &slave = slaves_[i];
    int parent_idx = slave.parent_index;
    if (parent_idx < 0)
      continue; // Should not happen for i > 0

    SlaveInfo &parent = slaves_[parent_idx];

    // Calculate internal delay in parent
    // Delay = ParentDelay + (TimeAtParentExit - TimeAtParentEntry) + WireDelay

    uint8_t pport = slave.parent_port;
    uint8_t p_entry = parent.entry_port;

    // We need the "previous port" to pport in the processing loop to find valid
    // time diff? Actually, strictly: TimeAtParentExit =
    // recv_times[parent][pport] TimeAtParentEntry = recv_times[parent][p_entry]
    // InternalDelay = TimeAtParentExit - TimeAtParentEntry

    // Note: recv_times are latched at the same instant (BWR).
    // Wait, BWR latches the *current local time* into the register.
    // So the difference represents the relative time the frame passed.
    // Yes.

    uint32_t t_parent_exit = recv_times[parent_idx][pport];
    uint32_t t_parent_entry = recv_times[parent_idx][p_entry];

    // Check for 32-bit wrap around? Usually times are close.
    int32_t internal_delay =
        static_cast<int32_t>(t_parent_exit - t_parent_entry);

    // Wire delay is unknown without return timestamp.
    // Standard assumption: ~5ns per meter? Or can be measured if loop is
    // closed. For open branching, we assume valid topology delay is mostly
    // internal + small wire. Let's add a small constant or 0.
    int32_t wire_delay = 0;

    slave.propagation_delay =
        parent.propagation_delay + internal_delay + wire_delay;
    slave.has_dc = true; // Mark as processed

    // Write delay to 0x0928
    write_register_fpwr<uint32_t>(slave.configured_address,
                                  regs::DC_SYS_TIME_DELAY,
                                  slave.propagation_delay);
  }
}

Result<> Enumerator::read_error_counters() {
  for (SlaveInfo &slave : slaves_) {
    int wkc;
    // Structure of ESC Error Counters (0x0300 - 0x0313)
    struct PackedErrorCounters {
      uint8_t rx_err_0;
      uint8_t inv_0;
      uint8_t rx_err_1;
      uint8_t inv_1;
      uint8_t rx_err_2;
      uint8_t inv_2;
      uint8_t rx_err_3;
      uint8_t inv_3;     // 0x307
      uint8_t fwd_rx_0;  // 0x308
      uint8_t fwd_rx_1;  // 0x309
      uint8_t fwd_rx_2;  // 0x30A
      uint8_t fwd_rx_3;  // 0x30B
      uint8_t proc_unit; // 0x30C
      uint8_t pdi;       // 0x30D
      uint8_t res1;
      uint8_t res2;        // 0x30E-0x30F Reserved
      uint8_t lost_link_0; // 0x310
      uint8_t lost_link_1; // 0x311
      uint8_t lost_link_2; // 0x312
      uint8_t lost_link_3; // 0x313
    } __attribute__((packed));

    PackedErrorCounters data = read_register_fprd<PackedErrorCounters>(
        slave.configured_address, regs::RX_ERR_COUNT_PORT0, wkc);

    if (wkc > 0) {
      slave.error_counters.rx_err_0 = data.rx_err_0;
      slave.error_counters.rx_err_1 = data.rx_err_1;
      slave.error_counters.rx_err_2 = data.rx_err_2;
      slave.error_counters.rx_err_3 = data.rx_err_3;

      slave.error_counters.fwd_rx_err_0 = data.fwd_rx_0;
      slave.error_counters.fwd_rx_err_1 = data.fwd_rx_1;
      slave.error_counters.fwd_rx_err_2 = data.fwd_rx_2;
      slave.error_counters.fwd_rx_err_3 = data.fwd_rx_3;

      slave.error_counters.proc_unit_err = data.proc_unit;
      slave.error_counters.pdi_err = data.pdi;

      slave.error_counters.lost_link_0 = data.lost_link_0;
      slave.error_counters.lost_link_1 = data.lost_link_1;
      slave.error_counters.lost_link_2 = data.lost_link_2;
      slave.error_counters.lost_link_3 = data.lost_link_3;
    }
  }
  return {};
}

} // namespace resoem
