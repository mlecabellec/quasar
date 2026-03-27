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
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>

namespace resoem {

Enumerator::Enumerator(RawSocket &socket) : socket_(socket) {}

Result<size_t> Enumerator::enumerate() {
  // Check if verbose logging is enabled and print start message.
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Starting network enumeration..." << std::endl;
  }
  // Clear the existing list of slaves to prepare for a fresh scan.
  slaves_.clear();

  // Reset the network to a known state (INIT, clear FMMUs/SMs, etc.).
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Resetting slaves to INIT and clearing ESC registers..." << std::endl;
  }
  // Issue broadcast commands to clear all slave controller states.
  reset_to_init();

  // Automatically detect and count slaves on the wire using broadcast read [FE-0040.3.1].
  int slave_count = broadcast_read_count();
  // Log the number of slaves found during the broadcast read.
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Broadcast read (BRD) found " << slave_count << " slaves on the bus." << std::endl;
  }
  
  // If no slaves are found, return zero immediately.
  if (slave_count <= 0) {
    return 0;
  }

  // Assign configured station addresses (starting at 0x1001) to each slave [FE-0040.3.2].
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Assigning station addresses starting at 0x1001..." << std::endl;
  }
  // Use auto-increment addressing to assign unique IDs to each discovered slave.
  assign_addresses(slave_count);

  // Parse information (Vendor, Product, PDOs) from the SII (EEPROM) of each slave [FE-0040.3.3].
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Reading SII (EEPROM) data for all slaves..." << std::endl;
  }
  // This step reads the identity and capabilities of every detected device.
  read_sii_data(slave_count);

  // Configure Mailbox SyncManagers (type 1 = Out, type 2 = In) before transitioning to PRE_OP.
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Configuring SyncManagers for Mailbox communication..." << std::endl;
  }
  // Define a packed structure for writing SM configuration to ESC registers.
  struct SMConfig {
    uint16_t start_addr;
    uint16_t length;
    uint32_t flags;
  } __attribute__((packed));

  // Loop through discovered slaves to program their SyncManagers.
  // We use a hard limit of 65535 iterations [CS-0010.37].
  for (size_t s_idx = 0; s_idx < slaves_.size() && s_idx < 65535; ++s_idx) {
    const SlaveInfo &info = slaves_[s_idx];
    if (verbose_level_ > 1) {
      std::cout << "[VERBOSE] Configuring Slave " << s_idx << " (" << info.name << ") SyncManagers..." << std::endl;
    }
    // Iterate through the SyncManagers identified during SII parsing.
    // We use a hard limit of 16 iterations as defined by ESC hardware limits.
    for (size_t i = 0; i < info.sync_managers.size() && i < 16; ++i) {
      const SyncManagerInfo &sm = info.sync_managers[i];
      // Only configure SyncManagers that are marked as valid mailbox or process data types.
      if (sm.type >= 1 && sm.type <= 4) {
        // Construct final_flags to ensure correct mailbox configuration.
        // We preserve the control byte (bits 0-7) and PDI control byte (bits 24-31),
        // and force the Enable bit (bit 16) in the Activate byte.
        uint32_t final_flags = (sm.flags & 0xFF0000FF) | 0x00010000;
        SMConfig cfg = {sm.start_addr, sm.length, final_flags};
        
        // Log detailed SM configuration if verbose mode is active.
        if (verbose_level_ > 1) {
          std::string type_name = "Unknown";
          if (sm.type == 1) type_name = "Mbx Out";
          else if (sm.type == 2) type_name = "Mbx In";
          else if (sm.type == 3) type_name = "Outputs";
          else if (sm.type == 4) type_name = "Inputs";
          
          std::cout << "  - SM" << i << " (" << type_name 
                    << "): Start=0x" << std::hex << std::setw(4) << std::setfill('0') << sm.start_addr 
                    << ", Len=" << std::dec << sm.length 
                    << ", Flags=0x" << std::hex << std::setw(8) << std::setfill('0') << final_flags << std::dec << std::endl;
        }

        // Write the configuration to the ESC SyncManager registers starting at 0x0800.
        uint16_t sm_reg = static_cast<uint16_t>(regs::SM0 + (i * 8));
        write_register_fpwr<SMConfig>(info.configured_address, sm_reg, cfg);
      }
    }
  }

  // Request all slaves to move to the PRE-OP state to enable mailbox communication.
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Requesting transition to PRE_OP for all slaves..." << std::endl;
  }
  // Transitioning to PRE-OP is mandatory before SDO/CoE operations can start.
  Result<> res = request_state_all(states::PRE_OP);
  if (!res) {
    if (verbose_level_ > 0) {
      std::cerr << "[VERBOSE] Failed to reach PRE_OP state: " << static_cast<int>(res.error()) << std::endl;
    }
    return std::unexpected(res.error());
  }

  // Return the total number of slaves successfully discovered and initialized.
  return static_cast<size_t>(slave_count);
}

Result<uint16_t> Enumerator::request_state(uint16_t slave_idx, uint16_t state,
                                           std::chrono::microseconds timeout) {
  if (slave_idx >= slaves_.size())
    return std::unexpected(ECError::ProtocolError);

  uint16_t cfg_addr = slaves_[slave_idx].configured_address;
  if (verbose_level_ > 0) {
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
        if (verbose_level_ > 1) {
          std::cout << "[VERBOSE] Slave " << slave_idx << ": Reached state 0x" 
                    << std::hex << cur << std::dec << "." << std::endl;
        }
        return cur;
      }

      // If the error bit is set, read the error code and try to acknowledge it.
      if (status & regs::al_status::ERROR_BIT) {
        uint16_t code =
            read_register_fprd<uint16_t>(cfg_addr, regs::AL_STATUS_CODE, wkc);
        if (verbose_level_ > 0) {
            std::cerr << "Slave " << slave_idx << " AL Status Error: 0x"
                      << std::hex << code << ": " << al_status_code_to_string(code)
                      << std::dec << std::endl;
        }
        
        if (verbose_level_ > 1) {
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
  if (verbose_level_ > 0) {
    std::cout << "[VERBOSE] Requesting state 0x" << std::hex << state 
              << std::dec << " for all slaves via broadcast..." << std::endl;
  }
  // Use a broadcast write to request state for all slaves at once.
  int wkc = write_register_broadcast<uint16_t>(regs::AL_CONTROL, state);
  if (wkc < (int)slaves_.size() && verbose_level_ > 0) {
      std::cerr << "[VERBOSE] Warning: AL_CONTROL broadcast WKC was " << wkc << ", expected " << slaves_.size() << std::endl;
  }

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
            if (verbose_level_ > 0) {
                std::cerr << "Slave " << j << " AL Status Error: 0x" << std::hex
                          << code << ": " << al_status_code_to_string(code)
                          << std::dec << std::endl;
            }
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
      if (verbose_level_ > 1) {
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
                    0, 0,
                    1,  // Type: Read (from physical)
                    1}; // Active

      // Find the appropriate SyncManager for outputs.
      std::vector<SyncManagerInfo>::const_iterator sm_it =
          std::find_if(info.sync_managers.begin(), info.sync_managers.end(),
                       [](const SyncManagerInfo &sm) { return sm.type == 3; });
      if (sm_it != info.sync_managers.end()) {
        fmmu.physical_start = sm_it->start_addr;
      } else {
          return std::unexpected(ECError::ProtocolError); // No output SM found
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
                    0, 0,
                    2,  // Type: Write (to physical)
                    1}; // Active

      // Find the appropriate SyncManager for inputs.
      std::vector<SyncManagerInfo>::const_iterator sm_it =
          std::find_if(info.sync_managers.begin(), info.sync_managers.end(),
                       [](const SyncManagerInfo &sm) { return sm.type == 4; });
      if (sm_it != info.sync_managers.end()) {
        fmmu.physical_start = sm_it->start_addr;
      } else {
          return std::unexpected(ECError::ProtocolError); // No input SM found
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

void Enumerator::configure_dc(const SlaveInfo &s, uint32_t cyc, int32_t shift) {
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
  // Force all slaves to INIT state first. This helps recover from lingering error states.
  // We write AL_CONTROL = 1 (INIT state) via broadcast.
  write_register_broadcast<uint16_t>(regs::AL_CONTROL, states::INIT);
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Give slaves time to react

  // Clear any existing aliases and request INIT state again with error
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

    // For all slaves, ensure DL control is initialized.
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
  try {
    socket_.send(frame);
  } catch (const SocketError &e) {
    if (verbose_) std::cerr << "[VERBOSE] Socket send error: " << e.what() << std::endl;
    return -1;
  }

  // Wait for the response with matching index.
  std::vector<byte> rx_buffer(1500);
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();

  // Use a reasonable loop count and always check the timer.
  for (uint64_t loop_count = 0; loop_count < 100000; ++loop_count) {
    int port_idx = 0;
    size_t received = socket_.receive(rx_buffer, &port_idx);

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

    if (received == 0) {
      if (elapsed_ms > 100) {
        return -1; // Timeout
      }
      continue;
    }

    // Basic size check: Eth(14) + ECat(2) + DgramHeader(10) + WKC(2) = 28
    if (received < 28) {
      if (elapsed_ms > 100) return -1;
      continue;
    }

    uint8_t received_idx = static_cast<uint8_t>(rx_buffer[17]);

    if (received_idx == idx) {
      size_t wkc_offset = 16 + 10 + data.size(); 
      if (received < wkc_offset + 2)
        return -2; // Fragmented

      uint16_t wkc;
      std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);

      // Improved Read command detection: 
      // APRD(1), APRW(3), FPRD(4), FPRW(6), BRD(7), BRW(9), LRD(10), LRW(12)
      bool is_read = (cmd == cmds::APRD || cmd == cmds::APRW || 
                      cmd == cmds::FPRD || cmd == cmds::FPRW || 
                      cmd == cmds::BRD  || cmd == cmds::BRW  ||
                      cmd == cmds::LRD  || cmd == cmds::LRW);

      if (wkc > 0 || is_read || (cmd == cmds::BWR))
        std::memcpy(data.data(), rx_buffer.data() + 16 + 10, data.size());

      return wkc;
    }

    if (elapsed_ms > 100) {
      return -1; // Timeout even if receiving other packets
    }
  }
  return -1;
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

Enumerator::SIICategory Enumerator::find_sii_category(uint16_t slave_cfg_addr,
                                              uint16_t cat_type) {
  // SII categories start at word 0x0040.
  // We initialize the pointer to this start address.
  uint16_t ptr = 0x0040;
  // Initialize result with zero values (not found).
  Enumerator::SIICategory result = {0, 0};

  // Iterate through the SII categories list.
  // We use a hard limit of 1000 iterations to avoid infinite loops [CS-0010.37].
  for (uint32_t loop_guard = 0; ptr < 0xFFFF && loop_guard < 1000;
       ++loop_guard) {
    // Read the category header (Type and Size).
    // Word address ptr contains Type in lower 16 bits, Size in upper 16 bits.
    uint32_t res = read_sii_word(slave_cfg_addr, ptr);
    // If read fails (returns 0xFFFFFFFF), we stop the search.
    if (res == 0xFFFFFFFF) {
      break;
    }

    // Extract category type from the lower 16 bits.
    uint16_t type = static_cast<uint16_t>(res & 0xFFFF);
    // If type is 0xFFFF, it indicates the end of the categories list.
    if (type == 0xFFFF) {
      break;
    }

    // Extract category size from the upper 16 bits.
    uint16_t size = static_cast<uint16_t>(res >> 16);
    // Check if this is the category we are looking for.
    if (type == cat_type) {
      // Data starts 1 word after the header.
      result.offset = ptr + 1;
      // Size of the data block in words.
      result.size_in_words = size;
      return result;
    }
    // Move to the next category: Header (1 word) + Data (size words).
    ptr += size + 1;

    // Check if we reached the hard limit [CS-0010.37].
    if (loop_guard == 999) {
      throw std::runtime_error("Hard limit reached in find_sii_category");
    }
  }
  // Return result (will be {0,0} if not found).
  return result;
}

std::string Enumerator::read_sii_string(uint16_t slave_cfg_addr,
                                        uint8_t string_idx) {
  // Return empty string if requested index is zero.
  if (string_idx == 0) {
    return "";
  }
  // Find the STRINGS category (Type 10).
  Enumerator::SIICategory cat = find_sii_category(slave_cfg_addr, 0x000A);
  // If category is not found (offset=0), return empty string.
  if (cat.offset == 0) {
    return "";
  }

  // Read the first word of the category to get the number of strings.
  uint32_t res = read_sii_word(slave_cfg_addr, cat.offset);
  // The first byte of the STRINGS category contains the string count.
  uint8_t num_strings = static_cast<uint8_t>(res & 0xFF);
  // If requested index exceeds available strings, return empty.
  if (string_idx > num_strings) {
    return "";
  }

  // Start iteration at the first string data word.
  uint16_t current_ptr = cat.offset;
  // Offset within the current word (0 or 1 bytes).
  uint8_t current_offset = 1;
  // Iterate through strings until the target index is reached.
  // We use a hard limit of 256 iterations [CS-0010.37].
  for (uint16_t i = 1; i <= string_idx && i < 256; ++i) {
    // Safety check to ensure we don't read past the category boundary.
    if (current_ptr >= cat.offset + cat.size_in_words) {
      break;
    }
    // Read the word containing the current string's length byte.
    uint32_t word_data = read_sii_word(slave_cfg_addr, current_ptr);
    // Extract length byte from the appropriate byte position.
    uint16_t word = static_cast<uint16_t>(word_data >> (8 * (current_offset % 2)));
    uint8_t len = static_cast<uint8_t>(word & 0xFF);

    // If this is the target string, extract the actual characters.
    if (i == string_idx) {
      std::string s;
      // Loop through each character of the string.
      // We use a hard limit of 256 iterations [CS-0010.37].
      for (uint16_t j = 0; j < len && j < 256; ++j) {
        // Calculate the byte offset relative to current_ptr.
        uint8_t byte_offset = static_cast<uint8_t>(current_offset + 1 + j);
        uint16_t target_ptr = static_cast<uint16_t>(current_ptr + (byte_offset / 2));
        // Ensure the character read is within category bounds.
        if (target_ptr >= cat.offset + cat.size_in_words) {
          break;
        }
        // Read the word containing the character at the calculated offset.
        uint32_t word_val = read_sii_word(slave_cfg_addr, target_ptr);
        // Append the extracted character byte to the resulting string.
        s += static_cast<char>((word_val >> (8 * (byte_offset % 2))) & 0xFF);
      }
      return s;
    }
    // Update the pointer and offset to skip the current string and its length byte.
    current_offset = static_cast<uint8_t>(current_offset + len + 1);
    current_ptr = static_cast<uint16_t>(current_ptr + (current_offset / 2));
    current_offset = static_cast<uint8_t>(current_offset % 2);
  }
  // Fallback return if the loop reaches hard limit or boundary.
  return "";
}

void Enumerator::read_sii_categories(int slave_idx) {
  // Access the target slave info from the internal list.
  SlaveInfo &info = slaves_[slave_idx];
  uint16_t addr = info.configured_address;

  // Read basic device information (Vendor ID and Product Code).
  // These are located at fixed SII word addresses 0x0008 and 0x000A.
  info.vendor_id = read_sii_word(addr, 0x0008);
  info.product_code = read_sii_word(addr, 0x000A);
  // Retrieve the device name string from SII (usually index 1).
  info.name = read_sii_string(addr, 1);

  // Read CoE details from the General category (Type 30).
  Enumerator::SIICategory gen_cat = find_sii_category(addr, 0x001E);
  // If General category exists and has sufficient length (at least 4 words).
  if (gen_cat.offset > 0 && gen_cat.size_in_words >= 4) {
    // Extract CoE details byte from word 3 of the General category.
    uint32_t gen_w3 = read_sii_word(addr, static_cast<uint16_t>(gen_cat.offset + 3));
    info.coe_details = static_cast<uint8_t>((gen_w3 >> 8) & 0xFF);
  }

  // Read SyncManager configurations from category Type 41 (0x0029).
  Enumerator::SIICategory sm_cat = find_sii_category(addr, 0x0029);
  // If SyncManager category is found.
  if (sm_cat.offset > 0) {
    // Calculate the number of SyncManager records (each record is 4 words / 8 bytes).
    uint16_t num_sm = static_cast<uint16_t>(sm_cat.size_in_words / 4);
    // Limit to 16 SMs to match hardware and internal list size.
    // We use a hard limit of 16 iterations [CS-0010.37].
    for (uint16_t i = 0; i < num_sm && i < 16; ++i) {
      // Read the 4 words comprising a single SyncManager record.
      uint32_t w12 = read_sii_word(addr, static_cast<uint16_t>(sm_cat.offset + i * 4));
      uint32_t w34 = read_sii_word(addr, static_cast<uint16_t>(sm_cat.offset + i * 4 + 2));
      // If the read fails, terminate the loop.
      if (w12 == 0xFFFFFFFF) {
        break;
      }

      // Initialize a temporary structure to hold SM info.
      SyncManagerInfo sm;
      // Start address is the lower 16 bits of the first word.
      sm.start_addr = static_cast<uint16_t>(w12 & 0xFFFF);
      // Length is the upper 16 bits of the first word.
      sm.length = static_cast<uint16_t>(w12 >> 16);
      // Flags are stored in the second 32-bit word.
      sm.flags = w34;

      // Validate the start address [SECURITY/SANITY CHECK].
      // Addresses below 0x1000 are usually reserved for ESC registers and are invalid for SMs.
      if (sm.start_addr < 0x1000 && sm.length > 0) {
        if (verbose_level_ > 0) {
          std::cerr << "Warning: Slave " << slave_idx << " has invalid SM" << i 
                    << " address 0x" << std::hex << sm.start_addr << std::dec << ". Skipping." << std::endl;
        }
        continue;
      }

      // Parse SM type and direction from flags (ESC SyncManager Control byte).
      // Bits 0-1: Operating Mode (00=Buffered, 10=Mailbox).
      // Bits 2-3: Direction (00=Master Read, 01=Master Write).
      uint8_t ctrl = static_cast<uint8_t>(sm.flags & 0xFF);
      bool is_mailbox = (ctrl & 0x03) == 0x02;
      bool is_write = ((ctrl >> 2) & 0x03) == 0x01;

      // Handle mailbox SyncManagers.
      if (is_mailbox) {
        if (is_write) {
          // Store mailbox output (Master -> Slave) parameters.
          info.mbx_out_offset = sm.start_addr;
          info.mbx_out_length = sm.length;
          sm.type = 1;
        } else {
          // Store mailbox input (Slave -> Master) parameters.
          info.mbx_in_offset = sm.start_addr;
          info.mbx_in_length = sm.length;
          sm.type = 2;
        }
      } else {
        // Handle Process Data SyncManagers.
        if (is_write) {
          sm.type = 3; // Outputs (Master -> Slave)
        } else {
          sm.type = 4; // Inputs (Slave -> Master)
        }
      }
      // Only add SM if it has a non-zero length or is a mandatory mailbox.
      if (sm.length == 0 && !is_mailbox) {
        continue;
      }
      // Add the validated SyncManager to the slave's list.
      info.sync_managers.push_back(sm);
    }
  }
}

void Enumerator::read_sii_pdos(int slave_idx) {
  // Access the target slave info from the internal list.
  SlaveInfo &info = slaves_[slave_idx];
  uint16_t addr = info.configured_address;

  // Lambda function to parse PDO categories (RxPDO or TxPDO).
  // Category Type 0x0032 (RxPDO) or 0x0033 (TxPDO).
  std::function<void(uint16_t, std::vector<PDOInfo> &)> parse_pdo_category =
      [&](uint16_t cat_type, std::vector<PDOInfo> &pdos) {
        // Find the PDO category in the SII.
        Enumerator::SIICategory cat = find_sii_category(addr, cat_type);
        // If category is not found, skip.
        if (cat.offset == 0) {
          return;
        }
        
        // Calculate the total size of the category data in words.
        uint16_t total_words = cat.size_in_words;
        // Pointer tracking our current position within the category.
        uint16_t cur_word = 0;
        
        // Iterate through PDO records within the category boundary.
        // We use a hard limit of 1000 iterations to avoid infinite loops [CS-0010.37].
        for (uint16_t loop_guard = 0; cur_word < total_words && loop_guard < 1000;
             ++loop_guard) {
          // Read the first word of the PDO record.
          uint32_t w1 = read_sii_word(addr, static_cast<uint16_t>(cat.offset + cur_word));
          // If read fails, terminate the parsing.
          if (w1 == 0xFFFFFFFF) {
            break;
          }
          
          PDOInfo pdo;
          // Extract PDO index from the lower 16 bits.
          pdo.index = static_cast<uint16_t>(w1 & 0xFFFF);
          // Number of entries in this PDO.
          uint8_t num_entries = static_cast<uint8_t>((w1 >> 16) & 0xFF);
          // SyncManager index this PDO is assigned to.
          pdo.sync_manager = static_cast<uint8_t>((w1 >> 24) & 0xFF);
          
          // Move pointer past the PDO header (4 words / 8 bytes).
          cur_word = static_cast<uint16_t>(cur_word + 4);
          
          // Parse each PDO entry associated with this PDO.
          // We use a hard limit of 256 iterations [CS-0010.37].
          for (uint16_t i = 0; i < num_entries && i < 256; ++i) {
            // Check if we would read past the category boundary.
            if (cur_word + 4 > total_words) {
              break;
            }
            
            PDOEntryInfo entry;
            // Read words containing entry details.
            uint32_t ew1 = read_sii_word(addr, static_cast<uint16_t>(cat.offset + cur_word));
            uint32_t ew2 = read_sii_word(addr, static_cast<uint16_t>(cat.offset + cur_word + 2));
            
            // Extract index and subindex.
            entry.index = static_cast<uint16_t>(ew1 & 0xFFFF);
            entry.subindex = static_cast<uint8_t>((ew1 >> 16) & 0xFF);
            // Extract bit length.
            entry.bit_length = static_cast<uint8_t>((ew2 >> 8) & 0xFF);
            
            // Add entry to the current PDO.
            pdo.entries.push_back(entry);
            // Each entry record is 4 words / 8 bytes.
            cur_word = static_cast<uint16_t>(cur_word + 4);
          }
          // Add completed PDO to the slave's list.
          pdos.push_back(pdo);
          
          // Check if we reached the hard limit [CS-0010.37].
          if (loop_guard == 999) {
            throw std::runtime_error("Hard limit reached in read_sii_pdos loop");
          }
        }
      };

  // Parse RxPDOs (Category Type 50 / 0x0032).
  parse_pdo_category(eeprom::CAT_PDO_RX, info.rx_pdos);
  // Parse TxPDOs (Category Type 51 / 0x0033).
  parse_pdo_category(eeprom::CAT_PDO_TX, info.tx_pdos);
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

  if (verbose_level_ > 0) std::cout << "Loading ENI configuration..." << std::endl;
  MailboxHandler mbx(socket_);
  mbx.set_verbose(verbose_level_);
  CoEHandler coe_handler(mbx);
  coe_handler.set_verbose(verbose_level_);

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

    if (verbose_level_ > 1) {
        std::cout << "Configuring Slave " << i << " (" << info.name << ")..."
                  << std::endl;
    }

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

// Explicit template instantiations to fix linker errors
template uint8_t Enumerator::read_register_broadcast<uint8_t>(uint16_t, int&);
template uint16_t Enumerator::read_register_broadcast<uint16_t>(uint16_t, int&);
template uint32_t Enumerator::read_register_broadcast<uint32_t>(uint16_t, int&);
template uint64_t Enumerator::read_register_broadcast<uint64_t>(uint16_t, int&);

template int Enumerator::write_register_broadcast<uint8_t>(uint16_t, const uint8_t&);
template int Enumerator::write_register_broadcast<uint16_t>(uint16_t, const uint16_t&);
template int Enumerator::write_register_broadcast<uint32_t>(uint16_t, const uint32_t&);
template int Enumerator::write_register_broadcast<uint64_t>(uint16_t, const uint64_t&);

template int Enumerator::write_register_apwr<uint8_t>(uint16_t, uint16_t, const uint8_t&);
template int Enumerator::write_register_apwr<uint16_t>(uint16_t, uint16_t, const uint16_t&);
template int Enumerator::write_register_apwr<uint32_t>(uint16_t, uint16_t, const uint32_t&);
template int Enumerator::write_register_apwr<uint64_t>(uint16_t, uint16_t, const uint64_t&);

template uint8_t Enumerator::read_register_fprd<uint8_t>(uint16_t, uint16_t, int&);
template uint16_t Enumerator::read_register_fprd<uint16_t>(uint16_t, uint16_t, int&);
template uint32_t Enumerator::read_register_fprd<uint32_t>(uint16_t, uint16_t, int&);
template uint64_t Enumerator::read_register_fprd<uint64_t>(uint16_t, uint16_t, int&);

template int Enumerator::write_register_fpwr<uint8_t>(uint16_t, uint16_t, const uint8_t&);
template int Enumerator::write_register_fpwr<uint16_t>(uint16_t, uint16_t, const uint16_t&);
template int Enumerator::write_register_fpwr<uint32_t>(uint16_t, uint16_t, const uint32_t&);
template int Enumerator::write_register_fpwr<uint64_t>(uint16_t, uint16_t, const uint64_t&);

} // namespace resoem
