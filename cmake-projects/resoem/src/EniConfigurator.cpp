/**
 * @file EniConfigurator.cpp
 * @brief Implementation of network configuration engine executing ETG.2100 ENI rules.
 * @details Contributes to [TSK-20260920-001.2].
 */

#include "resoem/EniConfigurator.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

namespace resoem::eni {

namespace {

constexpr std::size_t MAX_RETRY_LIMIT = 50;
constexpr std::size_t MAX_SLAVE_LIMIT = 4096;

} // anonymous namespace

EniConfigurator::EniConfigurator(IRawSocket &socket, const EniDocument &document)
    : m_socket{socket}, m_document{document} {}

std::size_t EniConfigurator::slave_count() const noexcept {
  return m_document.slaves().size();
}

const TransitionReport &EniConfigurator::last_report() const noexcept {
  return m_lastReport;
}

void EniConfigurator::set_abort_on_mandatory_failure(bool abort) noexcept {
  m_abortOnMandatory = abort;
}

void EniConfigurator::set_max_retries(std::uint32_t retries) noexcept {
  m_defaultRetries = retries;
}

bool EniConfigurator::matches_transition(EsmTransition cmd_transition,
                                         EsmTransition target) noexcept {
  return (cmd_transition & target) != EsmTransition::NONE;
}

int EniConfigurator::send_receive_datagram(std::uint8_t cmd, std::uint16_t addr,
                                          std::uint16_t offset, std::span<byte> data) {
  FrameBuilder builder;
  std::uint8_t idx = m_currentIdx++;
  builder.add_datagram(cmd, idx, addr, offset, data);
  const std::span<const byte> TX_FRAME = builder.build();

  try {
    m_socket.send(TX_FRAME);
  } catch (const SocketError &) {
    return -1;
  }

  std::vector<byte> rx_buffer{};
  rx_buffer.assign(MAX_FRAME_SIZE, static_cast<byte>(0));
  std::size_t received = 0;
  try {
    received = m_socket.receive(rx_buffer);
  } catch (const SocketError &) {
    return -1;
  }

  if (received < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + data.size() + WORKING_COUNTER_SIZE) {
    return -1;
  } else {
    // Valid frame received
  }

  std::size_t payload_offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE;
  std::size_t wkc_offset = payload_offset + data.size();

  // Copy received payload back to caller
  std::memcpy(data.data(), &rx_buffer[payload_offset], data.size());

  std::uint16_t wkc = 0;
  std::memcpy(&wkc, &rx_buffer[wkc_offset], sizeof(std::uint16_t));

  return static_cast<int>(wkc);
}

bool EniConfigurator::validate_data(std::span<const byte> received,
                                    const ValidateRule &rule) const {
  if (rule.data.empty()) {
    return true;
  } else {
    // Check byte by byte
  }

  for (std::size_t i = 0; i < rule.data.size(); ++i) {
    if (i >= received.size()) {
      return false;
    } else {
      // Within bounds
    }

    std::uint8_t mask = (i < rule.data_mask.size()) ? rule.data_mask[i] : 0xFF;
    if ((received[i] & mask) != (rule.data[i] & mask)) {
      return false;
    } else {
      // Byte matches
    }
  }

  return true;
}

bool EniConfigurator::execute_single_init_cmd(const EniInitCmd &cmd, std::size_t slave_idx,
                                              bool is_optional) {
  std::vector<byte> payload_copy = cmd.data;
  std::span<byte> payload_span{payload_copy.data(), payload_copy.size()};

  std::uint32_t max_retries = std::max(cmd.retries, m_defaultRetries);
  if (max_retries > MAX_RETRY_LIMIT) {
    max_retries = MAX_RETRY_LIMIT;
  } else {
    // within limit
  }

  std::uint8_t cmd_code = static_cast<std::uint8_t>(cmd.cmd);
  std::uint16_t addr = cmd.adp;
  std::uint16_t offset = cmd.ado;

  bool validated = false;
  for (std::uint32_t attempt = 0; attempt <= max_retries; ++attempt) {
    int wkc = send_receive_datagram(cmd_code, addr, offset, payload_span);
    if (wkc >= 0) {
      if (cmd.validate.has_value()) {
        if (validate_data(payload_span, cmd.validate.value())) {
          validated = true;
          break;
        } else {
          // Validation condition not yet satisfied
        }
      } else {
        validated = true;
        break;
      }
    } else {
      // Transmission failed
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  if (!validated) {
    m_lastReport.success = false;
    m_lastReport.failed_slave_idx = slave_idx;
    m_lastReport.failed_command_comment = cmd.comment;
    m_lastReport.error_message = "InitCmd validation failed for slave " + std::to_string(slave_idx) +
                                 ": " + cmd.comment;

    if (!is_optional && m_abortOnMandatory) {
      return false;
    } else {
      // Optional slave: ignore failure and proceed
      return true;
    }
  } else {
    return true;
  }
}

Result<void> EniConfigurator::configure_master_init_cmds(EsmTransition transition) {
  for (const EniInitCmd &cmd : m_document.master().init_cmds) {
    if (matches_transition(cmd.transition, transition)) {
      if (!execute_single_init_cmd(cmd, 0, false)) {
        return std::unexpected(ECError::ProtocolError);
      } else {
        // Proceed
      }
    } else {
      // Skip cmd not matching transition
    }
  }
  return {};
}

Result<void> EniConfigurator::configure_slave_init_cmds(std::size_t slave_idx,
                                                       EsmTransition transition) {
  if (slave_idx >= m_document.slaves().size()) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid index
  }

  const EniSlave &slave = m_document.slaves()[slave_idx];
  bool is_optional = slave.hot_connect.has_value();

  for (const EniInitCmd &cmd : slave.init_cmds) {
    if (matches_transition(cmd.transition, transition)) {
      if (!execute_single_init_cmd(cmd, slave_idx, is_optional)) {
        return std::unexpected(ECError::ProtocolError);
      } else {
        // Proceed
      }
    } else {
      // Skip cmd not matching transition
    }
  }

  return {};
}

Result<void> EniConfigurator::configure_fmmu_and_sm(std::size_t slave_idx) {
  if (slave_idx >= m_document.slaves().size()) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid index
  }

  const EniSlave &slave = m_document.slaves()[slave_idx];
  std::uint16_t cfg_addr = slave.phys_addr;

  // 1. Program SyncManagers
  for (std::size_t k = 0; k < slave.sync_managers.size() && k < 16; ++k) {
    const EniSyncManager &sm = slave.sync_managers[k];
    std::uint16_t sm_reg = regs::SM0 + static_cast<std::uint16_t>(k * 8);

    std::array<byte, 8> sm_buf{};
    std::memcpy(&sm_buf[0], &sm.start_address, sizeof(std::uint16_t));
    std::memcpy(&sm_buf[2], &sm.length, sizeof(std::uint16_t));
    sm_buf[4] = sm.control_byte;
    sm_buf[5] = 0;
    sm_buf[6] = sm.enable ? 1 : 0;
    sm_buf[7] = 0;

    int wkc = send_receive_datagram(cmds::FPWR, cfg_addr, sm_reg, sm_buf);
    if (wkc <= 0 && !slave.hot_connect.has_value() && m_abortOnMandatory) {
      return std::unexpected(ECError::ProtocolError);
    } else {
      // SM programmed
    }
  }

  // 2. Program FMMUs for Outputs and Inputs if PDOs present
  if (!slave.rx_pdos.empty()) {
    std::uint16_t sm_start = 0x0F00;
    std::uint16_t sm_len = 1;
    for (const EniSyncManager &sm : slave.sync_managers) {
      if (sm.type == "Outputs") {
        sm_start = sm.start_address;
        sm_len = sm.length;
        break;
      } else {
        // continue search
      }
    }

    std::array<byte, 16> fmmu_buf{};
    std::uint32_t log_start = 0;
    std::memcpy(&fmmu_buf[0], &log_start, sizeof(std::uint32_t));
    std::memcpy(&fmmu_buf[4], &sm_len, sizeof(std::uint16_t));
    fmmu_buf[6] = 0;
    fmmu_buf[7] = 7;
    std::memcpy(&fmmu_buf[8], &sm_start, sizeof(std::uint16_t));
    fmmu_buf[10] = 0;
    fmmu_buf[11] = 0x02; // Write (Outputs)
    fmmu_buf[12] = 1; // Activate

    (void)send_receive_datagram(cmds::FPWR, cfg_addr, regs::FMMU0, fmmu_buf);
  } else {
    // No RxPDOs
  }

  if (!slave.tx_pdos.empty()) {
    std::uint16_t sm_start = 0x0F00;
    std::uint16_t sm_len = 1;
    for (const EniSyncManager &sm : slave.sync_managers) {
      if (sm.type == "Inputs") {
        sm_start = sm.start_address;
        sm_len = sm.length;
        break;
      } else {
        // continue search
      }
    }

    std::array<byte, 16> fmmu_buf{};
    std::uint32_t log_start = 1;
    std::memcpy(&fmmu_buf[0], &log_start, sizeof(std::uint32_t));
    std::memcpy(&fmmu_buf[4], &sm_len, sizeof(std::uint16_t));
    fmmu_buf[6] = 0;
    fmmu_buf[7] = 7;
    std::memcpy(&fmmu_buf[8], &sm_start, sizeof(std::uint16_t));
    fmmu_buf[10] = 0;
    fmmu_buf[11] = 0x01; // Read (Inputs)
    fmmu_buf[12] = 1; // Activate

    (void)send_receive_datagram(cmds::FPWR, cfg_addr, regs::FMMU1, fmmu_buf);
  } else {
    // No TxPDOs
  }

  return {};
}

Result<void> EniConfigurator::configure_dc(std::size_t slave_idx) {
  if (slave_idx >= m_document.slaves().size()) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid index
  }

  const EniSlave &slave = m_document.slaves()[slave_idx];
  std::uint16_t cfg_addr = slave.phys_addr;

  if (slave.dc.cycle_time_0 > 0) {
    std::array<byte, 4> cyc0{};
    std::memcpy(cyc0.data(), &slave.dc.cycle_time_0, sizeof(std::uint32_t));
    (void)send_receive_datagram(cmds::FPWR, cfg_addr, regs::DC_SYNC0_CYCLE_TIME, cyc0);
  } else {
    // No DC Sync0
  }

  return {};
}

bool EniConfigurator::request_slave_state(std::size_t slave_idx, std::uint16_t target_state,
                                         std::chrono::milliseconds timeout) {
  if (slave_idx >= m_document.slaves().size()) {
    return false;
  } else {
    // Valid index
  }

  const EniSlave &slave = m_document.slaves()[slave_idx];
  std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + timeout;

  // Send FPWR to AL_CONTROL (0x0120)
  std::array<byte, 2> al_ctrl_buf{};
  std::memcpy(al_ctrl_buf.data(), &target_state, sizeof(std::uint16_t));
  (void)send_receive_datagram(cmds::FPWR, slave.phys_addr, regs::AL_CONTROL, al_ctrl_buf);

  // Poll AL_STATUS (0x0130)
  for (std::size_t iter = 0; iter < 100; ++iter) {
    std::array<byte, 2> al_stat_buf{};
    int wkc = send_receive_datagram(cmds::FPRD, slave.phys_addr, regs::AL_STATUS, al_stat_buf);
    if (wkc > 0) {
      std::uint16_t actual_state = 0;
      std::memcpy(&actual_state, al_stat_buf.data(), sizeof(std::uint16_t));
      if ((actual_state & resoem::regs::al_status::STATE_MASK) == target_state) {
        return true;
      } else {
        // Still transitioning
      }
    } else {
      // Read failed
    }

    if (std::chrono::steady_clock::now() > deadline) {
      break;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  return false;
}

bool EniConfigurator::transition_slaves_to_state(std::uint16_t target_state,
                                                 std::chrono::milliseconds timeout) {
  // Request target state for each slave
  for (const EniSlave &slave : m_document.slaves()) {
    std::array<byte, 2> al_ctrl_buf{};
    std::memcpy(al_ctrl_buf.data(), &target_state, sizeof(std::uint16_t));
    (void)send_receive_datagram(cmds::FPWR, slave.phys_addr, regs::AL_CONTROL, al_ctrl_buf);
  }

  // Poll until slaves reach target state
  for (std::size_t i = 0; i < m_document.slaves().size(); ++i) {
    const EniSlave &slave = m_document.slaves()[i];
    bool reached = request_slave_state(i, target_state, timeout);

    if (!reached && !slave.hot_connect.has_value() && m_abortOnMandatory) {
      m_lastReport.success = false;
      m_lastReport.failed_slave_idx = i;
      m_lastReport.error_message = "Slave " + slave.name + " failed to reach target state 0x" +
                                   std::to_string(target_state);
      return false;
    } else {
      // Reached or optional
    }
  }

  return true;
}

Result<void> EniConfigurator::run_transition(EsmTransition transition) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(500)};
  if (!lock.owns_lock()) {
    return std::unexpected(ECError::Timeout);
  } else {
    // Lock acquired
  }

  if (transition == EsmTransition::IP) {
    // 1. Master InitCmds (reset bus, DL control, etc.)
    Result<void> master_res = configure_master_init_cmds(transition);
    if (!master_res.has_value()) {
      return master_res;
    } else {
      // Master commands passed
    }

    // 2. Assign Configured Station Addresses
    for (std::size_t i = 0; i < m_document.slaves().size() && i < MAX_SLAVE_LIMIT; ++i) {
      const EniSlave &slave = m_document.slaves()[i];
      std::uint16_t auto_inc_addr = static_cast<std::uint16_t>(-static_cast<int>(i));
      std::uint16_t cfg_addr = slave.phys_addr;

      std::array<byte, 2> addr_buf{};
      std::memcpy(addr_buf.data(), &cfg_addr, sizeof(std::uint16_t));
      int wkc = send_receive_datagram(cmds::APWR, auto_inc_addr, regs::CONFIG_STATION_ADDR, addr_buf);
      if (wkc <= 0 && !slave.hot_connect.has_value() && m_abortOnMandatory) {
        return std::unexpected(ECError::ProtocolError);
      } else {
        // Station address assigned
      }
    }

    // 3. Slave IP InitCmds
    for (std::size_t i = 0; i < m_document.slaves().size(); ++i) {
      Result<void> slave_res = configure_slave_init_cmds(i, transition);
      if (!slave_res.has_value()) {
        return slave_res;
      } else {
        // Passed
      }
    }

    // 4. Transition to PRE_OP
    if (!transition_slaves_to_state(states::PRE_OP, std::chrono::milliseconds(DEFAULT_TRANSITION_TIMEOUT_MS))) {
      return std::unexpected(ECError::Timeout);
    } else {
      // PRE_OP reached
    }
  } else if (transition == EsmTransition::PS) {
    // 1. Slave PS InitCmds (CoE SDOs, etc.)
    for (std::size_t i = 0; i < m_document.slaves().size(); ++i) {
      Result<void> slave_res = configure_slave_init_cmds(i, transition);
      if (!slave_res.has_value()) {
        return slave_res;
      } else {
        // Passed
      }

      // 2. Program FMMUs and SyncManagers
      Result<void> fmmu_res = configure_fmmu_and_sm(i);
      if (!fmmu_res.has_value()) {
        return fmmu_res;
      } else {
        // FMMU & SM programmed
      }

      // 3. Program DC
      (void)configure_dc(i);
    }

    // 4. Transition to SAFE_OP
    if (!transition_slaves_to_state(states::SAFE_OP, std::chrono::milliseconds(DEFAULT_TRANSITION_TIMEOUT_MS))) {
      return std::unexpected(ECError::Timeout);
    } else {
      // SAFE_OP reached
    }
  } else if (transition == EsmTransition::SO) {
    // 1. Slave SO InitCmds
    for (std::size_t i = 0; i < m_document.slaves().size(); ++i) {
      Result<void> slave_res = configure_slave_init_cmds(i, transition);
      if (!slave_res.has_value()) {
        return slave_res;
      } else {
        // Passed
      }
    }

    // 2. Transition to OP
    if (!transition_slaves_to_state(states::OP, std::chrono::milliseconds(DEFAULT_TRANSITION_TIMEOUT_MS))) {
      return std::unexpected(ECError::Timeout);
    } else {
      // OP reached
    }
  } else {
    // Other transitions (PI, SP, etc.)
  }

  return {};
}

Result<void> EniConfigurator::configure_all() {
  Result<void> ip_res = run_transition(EsmTransition::IP);
  if (!ip_res.has_value()) {
    return ip_res;
  } else {
    // IP succeeded
  }

  Result<void> ps_res = run_transition(EsmTransition::PS);
  if (!ps_res.has_value()) {
    return ps_res;
  } else {
    // PS succeeded
  }

  Result<void> so_res = run_transition(EsmTransition::SO);
  if (!so_res.has_value()) {
    return so_res;
  } else {
    // SO succeeded
  }

  m_lastReport.success = true;
  return {};
}

Result<std::uint16_t> EniConfigurator::exchange_cyclic_frame(std::size_t frame_idx,
                                                             std::span<byte> process_image_data) {
  std::unique_lock<std::timed_mutex> lock{m_mutex, std::chrono::milliseconds(200)};
  if (!lock.owns_lock()) {
    return std::unexpected(ECError::Timeout);
  } else {
    // Lock acquired
  }

  if (frame_idx >= m_document.cyclic_frames().size()) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Valid frame index
  }

  const EniCyclicFrame &frame_cfg = m_document.cyclic_frames()[frame_idx];
  FrameBuilder builder;
  std::uint8_t idx = m_currentIdx++;

  for (const EniCyclicCmd &cmd : frame_cfg.commands) {
    std::size_t data_len = cmd.data_length;
    std::vector<byte> cmd_payload{};
    cmd_payload.assign(data_len, static_cast<byte>(0));

    if (cmd.addr < process_image_data.size()) {
      std::size_t copy_bytes = std::min(data_len, process_image_data.size() - cmd.addr);
      std::memcpy(cmd_payload.data(), &process_image_data[cmd.addr], copy_bytes);
    } else {
      // Out of bounds
    }

    builder.add_datagram_logical(static_cast<std::uint8_t>(cmd.cmd), idx, cmd.addr, cmd_payload);
  }

  const std::span<const byte> TX_FRAME = builder.build();
  try {
    m_socket.send(TX_FRAME);
  } catch (const SocketError &) {
    return std::unexpected(ECError::ProtocolError);
  }

  std::vector<byte> rx_buffer{};
  rx_buffer.assign(MAX_FRAME_SIZE, static_cast<byte>(0));
  std::size_t received = 0;
  try {
    received = m_socket.receive(rx_buffer);
  } catch (const SocketError &) {
    return std::unexpected(ECError::ProtocolError);
  }

  if (received < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE) {
    return std::unexpected(ECError::InvalidResponse);
  } else {
    // Process received datagrams
  }

  std::size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  std::uint16_t total_wkc = 0;

  for (std::size_t i = 0; i < frame_cfg.commands.size(); ++i) {
    const EniCyclicCmd &cmd = frame_cfg.commands[i];
    if (offset + DATAGRAM_HEADER_SIZE + cmd.data_length + WORKING_COUNTER_SIZE > received) {
      break;
    } else {
      // Fits in frame
    }

    // Copy read payload back into process_image_data
    if (cmd.addr < process_image_data.size()) {
      std::size_t copy_bytes = std::min(cmd.data_length, process_image_data.size() - cmd.addr);
      std::memcpy(&process_image_data[cmd.addr], &rx_buffer[offset + DATAGRAM_HEADER_SIZE], copy_bytes);
    } else {
      // Out of range
    }

    std::uint16_t dgram_wkc = 0;
    std::memcpy(&dgram_wkc, &rx_buffer[offset + DATAGRAM_HEADER_SIZE + cmd.data_length], sizeof(std::uint16_t));
    total_wkc += dgram_wkc;

    offset += DATAGRAM_HEADER_SIZE + cmd.data_length + WORKING_COUNTER_SIZE;
  }

  return total_wkc;
}

} // namespace resoem::eni
