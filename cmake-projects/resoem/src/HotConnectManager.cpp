/**
 * @file HotConnectManager.cpp
 * @brief Implementation of EtherCAT Hot Connect dynamic group manager.
 * @details Contributes to [TSK-20260920-001.8], [TSK-20260920-001.9], and [FE-0040.8].
 */

#include "resoem/HotConnectManager.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>

namespace resoem::hot_connect {

namespace {

constexpr std::uint16_t DEFAULT_HEAD_SLAVE_WKC = 1;
constexpr std::size_t MAX_INIT_RETRIES = 3;

} // anonymous namespace

HotConnectManager::HotConnectManager(const eni::EniDocument &doc, IRawSocket &socket,
                                     eni::EniConfigurator &configurator)
    : m_doc(doc), m_socket(socket), m_configurator(configurator) {
  // Compute initial base expected WKC from cyclic frames
  std::uint16_t cyc_wkc = 0;
  for (const eni::EniCyclicFrame &cf : m_doc.cyclic_frames()) {
    for (const eni::EniCyclicCmd &cmd : cf.commands) {
      cyc_wkc += cmd.expected_wkc;
    }
  }
  m_baseExpectedWkc = cyc_wkc;
}

void HotConnectManager::register_group(const HotConnectGroupConfig &config) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    // Deduct group's expected WKC from base expected WKC if group is currently disconnected
    if (config.state == HotConnectGroupState::DISCONNECTED && m_baseExpectedWkc >= config.expected_wkc) {
      m_baseExpectedWkc -= config.expected_wkc;
    } else {
      // Base already adjusted
    }
    m_groups.push_back(config);
  } else {
    // Lock failed
  }
}

void HotConnectManager::auto_discover_groups_from_eni() {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return;
  } else {
    // Lock acquired
  }

  const std::vector<eni::EniSlave> &slaves = m_doc.slaves();
  for (std::size_t i = 0; i < slaves.size(); ++i) {
    const eni::EniSlave &slv = slaves[i];
    if (slv.hot_connect.has_value()) {
      const std::string &grp_name = slv.hot_connect->group_name;

      // Find existing group or create new
      HotConnectGroupConfig *target_grp = nullptr;
      for (HotConnectGroupConfig &grp : m_groups) {
        if (grp.name == grp_name) {
          target_grp = &grp;
          break;
        } else {
          // continue search
        }
      }

      if (target_grp == nullptr) {
        HotConnectGroupConfig new_grp;
        new_grp.name = grp_name;
        new_grp.station_alias = slv.phys_addr; // default to phys_addr or identify cmd
        new_grp.state = HotConnectGroupState::DISCONNECTED;

        // Extract expected station alias from identify commands if present
        for (const eni::EniInitCmd &cmd : slv.hot_connect->identify_cmds) {
          if (cmd.ado == regs::CONFIG_STATION_ALIAS && cmd.validate.has_value() &&
              cmd.validate->data.size() >= 2) {
            std::uint16_t al = 0;
            std::memcpy(&al, cmd.validate->data.data(), sizeof(std::uint16_t));
            new_grp.station_alias = al;
          } else {
            // Other identify cmd
          }
        }

        // Expected WKC: count RxPDOs and TxPDOs
        std::uint16_t wkc = 0;
        if (!slv.rx_pdos.empty()) {
          wkc += 1;
        } else {
          // No outputs
        }
        if (!slv.tx_pdos.empty()) {
          wkc += 1;
        } else {
          // No inputs
        }
        if (wkc == 0) {
          wkc = DEFAULT_HEAD_SLAVE_WKC;
        } else {
          // Valid PDO WKC
        }
        new_grp.expected_wkc = wkc;
        new_grp.slave_indices.push_back(i);

        // Deduct from base expected WKC
        if (m_baseExpectedWkc >= new_grp.expected_wkc) {
          m_baseExpectedWkc -= new_grp.expected_wkc;
        } else {
          // No deduction
        }

        m_groups.push_back(new_grp);
      } else {
        // Group exists: append slave index and update WKC
        target_grp->slave_indices.push_back(i);
        std::uint16_t additional_wkc = 0;
        if (!slv.rx_pdos.empty()) {
          additional_wkc += 1;
        } else {
          // No outputs
        }
        if (!slv.tx_pdos.empty()) {
          additional_wkc += 1;
        } else {
          // No inputs
        }
        if (additional_wkc == 0) {
          additional_wkc = DEFAULT_HEAD_SLAVE_WKC;
        } else {
          // Valid PDO WKC
        }
        target_grp->expected_wkc += additional_wkc;
        if (m_baseExpectedWkc >= additional_wkc) {
          m_baseExpectedWkc -= additional_wkc;
        } else {
          // No deduction
        }
      }
    } else {
      // Mandatory slave (not hot connect)
    }
  }
}

HotConnectGroupState HotConnectManager::group_state(const std::string &name) const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return HotConnectGroupState::ERROR_STATE;
  } else {
    for (const HotConnectGroupConfig &grp : m_groups) {
      if (grp.name == name) {
        return grp.state;
      } else {
        // continue search
      }
    }
    return HotConnectGroupState::ERROR_STATE;
  }
}

void HotConnectManager::set_group_policy(const std::string &name, DisconnectedDataPolicy policy) {
  set_group_policy(name, policy, std::span<const byte>{});
}

void HotConnectManager::set_group_policy(const std::string &name, DisconnectedDataPolicy policy,
                                         std::span<const byte> fallback_data) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    for (HotConnectGroupConfig &grp : m_groups) {
      if (grp.name == name) {
        grp.disconnected_policy = policy;
        if (policy == DisconnectedDataPolicy::CUSTOM_FALLBACK) {
          grp.custom_fallback_data.assign(fallback_data.begin(), fallback_data.end());
        } else {
          grp.custom_fallback_data.clear();
        }
        break;
      } else {
        // continue search
      }
    }
  } else {
    // Lock failed
  }
}

std::uint16_t HotConnectManager::calculate_active_expected_wkc() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return m_baseExpectedWkc;
  } else {
    std::uint16_t total = m_baseExpectedWkc;
    for (const HotConnectGroupConfig &grp : m_groups) {
      if (grp.state == HotConnectGroupState::OPERATIONAL) {
        total += grp.expected_wkc;
      } else {
        // Disconnected or transitioning: do not count WKC
      }
    }
    return total;
  }
}

void HotConnectManager::apply_disconnected_data_policies(std::span<byte> process_image) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return;
  } else {
    for (const HotConnectGroupConfig &grp : m_groups) {
      if (grp.state != HotConnectGroupState::OPERATIONAL) {
        // Enforce input boundary checks before applying policy
        if (grp.input_process_length > 0 &&
            grp.input_process_offset + grp.input_process_length <= process_image.size()) {
          if (grp.disconnected_policy == DisconnectedDataPolicy::ZERO_FILL) {
            // Apply zero-fill policy for safe state
            std::memset(&process_image[grp.input_process_offset], 0x00, grp.input_process_length);
          } else if (grp.disconnected_policy == DisconnectedDataPolicy::HOLD_LAST_STATE) {
            // Hold last valid process data if available
            if (!grp.last_known_valid_data.empty()) {
              std::size_t copy_bytes = std::min(grp.input_process_length, grp.last_known_valid_data.size());
              std::memcpy(&process_image[grp.input_process_offset], grp.last_known_valid_data.data(), copy_bytes);
            } else {
              // Fallback to zero if no prior valid data recorded
              std::memset(&process_image[grp.input_process_offset], 0x00, grp.input_process_length);
            }
          } else if (grp.disconnected_policy == DisconnectedDataPolicy::CUSTOM_FALLBACK) {
            // Overwrite with pre-configured custom fallback bytes
            if (!grp.custom_fallback_data.empty()) {
              std::size_t copy_bytes = std::min(grp.input_process_length, grp.custom_fallback_data.size());
              std::memcpy(&process_image[grp.input_process_offset], grp.custom_fallback_data.data(), copy_bytes);
            } else {
              // Fallback to zero if no custom data provided
              std::memset(&process_image[grp.input_process_offset], 0x00, grp.input_process_length);
            }
          } else {
            // Default safe zero-fill
            std::memset(&process_image[grp.input_process_offset], 0x00, grp.input_process_length);
          }
        } else {
          // Out of bounds or length is 0
        }
      } else {
        // Group is operational: keep live process data
      }
    }
  }
}

void HotConnectManager::record_valid_inputs(const std::string &name, std::span<const byte> inputs) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    for (HotConnectGroupConfig &grp : m_groups) {
      if (grp.name == name) {
        grp.last_known_valid_data.assign(inputs.begin(), inputs.end());
        break;
      } else {
        // continue search
      }
    }
  } else {
    // Lock failed
  }
}

bool HotConnectManager::probe_and_identify_group(HotConnectGroupConfig &group) {
  if (group.slave_indices.empty()) {
    return false;
  } else {
    // Valid indices
  }

  std::size_t head_idx = group.slave_indices[0];
  if (head_idx >= m_doc.slaves().size()) {
    return false;
  } else {
    // Valid index
  }

  const eni::EniSlave &head_slave = m_doc.slaves()[head_idx];
  std::uint16_t addr = head_slave.phys_addr;

  // Send FPRD to head slave at CONFIG_STATION_ALIAS (0x0012)
  FrameBuilder builder;
  std::array<byte, 2> alias_buf{0x00, 0x00};
  builder.add_datagram(cmds::FPRD, 0x01, addr, regs::CONFIG_STATION_ALIAS, alias_buf);
  const std::span<const byte> TX_SPAN = builder.build();

  try {
    m_socket.send(TX_SPAN, false);
  } catch (const SocketError &) {
    return false;
  }

  std::vector<byte> rx_buffer(MAX_FRAME_SIZE, static_cast<byte>(0));
  std::size_t rec = 0;
  try {
    rec = m_socket.receive(rx_buffer, nullptr);
  } catch (const SocketError &) {
    return false;
  }

  if (rec < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + 2 + WORKING_COUNTER_SIZE) {
    return false;
  } else {
    // Frame received
  }

  std::size_t dgram_off = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  std::uint16_t wkc = 0;
  std::memcpy(&wkc, &rx_buffer[dgram_off + DATAGRAM_HEADER_SIZE + 2], sizeof(std::uint16_t));

  if (wkc >= 1) {
    std::uint16_t read_alias = 0;
    std::memcpy(&read_alias, &rx_buffer[dgram_off + DATAGRAM_HEADER_SIZE], sizeof(std::uint16_t));
    if (group.station_alias != 0 && read_alias == group.station_alias) {
      return true;
    } else if (group.station_alias == 0) {
      return true;
    } else {
      // Alias mismatch
      return false;
    }
  } else {
    return false;
  }
}

bool HotConnectManager::check_group_health(HotConnectGroupConfig &group) {
  if (group.slave_indices.empty()) {
    return false;
  } else {
    // Valid indices
  }

  std::size_t head_idx = group.slave_indices[0];
  if (head_idx >= m_doc.slaves().size()) {
    return false;
  } else {
    // Valid index
  }

  const eni::EniSlave &head_slave = m_doc.slaves()[head_idx];
  std::uint16_t addr = head_slave.phys_addr;

  // Probe AL_STATUS (0x0130) of head slave
  FrameBuilder builder;
  std::array<byte, 2> al_buf{0x00, 0x00};
  builder.add_datagram(cmds::FPRD, 0x01, addr, regs::AL_STATUS, al_buf);
  const std::span<const byte> TX_SPAN = builder.build();

  try {
    m_socket.send(TX_SPAN, false);
  } catch (const SocketError &) {
    return false;
  }

  std::vector<byte> rx_buf(MAX_FRAME_SIZE, static_cast<byte>(0));
  std::size_t rec = 0;
  try {
    rec = m_socket.receive(rx_buf, nullptr);
  } catch (const SocketError &) {
    return false;
  }

  if (rec < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + 2 + WORKING_COUNTER_SIZE) {
    return false;
  } else {
    // Frame received
  }

  std::size_t dgram_off = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  std::uint16_t wkc = 0;
  std::memcpy(&wkc, &rx_buf[dgram_off + DATAGRAM_HEADER_SIZE + 2], sizeof(std::uint16_t));
  return wkc >= 1;
}

bool HotConnectManager::initialize_group_non_intrusive(HotConnectGroupConfig &group) {
  group.state = HotConnectGroupState::INITIALIZING;

  // Perform targeted transitions through EniConfigurator for group slaves
  for (std::size_t idx : group.slave_indices) {
    if (idx >= m_doc.slaves().size()) {
      group.state = HotConnectGroupState::ERROR_STATE;
      return false;
    } else {
      // Valid index
    }

    // 1. IP InitCmds and PRE_OP
    (void)m_configurator.configure_slave_init_cmds(idx, eni::EsmTransition::IP);
    if (!m_configurator.request_slave_state(idx, states::PRE_OP)) {
      group.state = HotConnectGroupState::ERROR_STATE;
      return false;
    } else {
      // Reached PRE_OP
    }

    // 2. PS InitCmds, FMMU/SM, and SAFE_OP
    (void)m_configurator.configure_slave_init_cmds(idx, eni::EsmTransition::PS);
    (void)m_configurator.configure_fmmu_and_sm(idx);
    if (!m_configurator.request_slave_state(idx, states::SAFE_OP)) {
      group.state = HotConnectGroupState::ERROR_STATE;
      return false;
    } else {
      // Reached SAFE_OP
    }

    // 3. SO InitCmds, DC, and OP
    (void)m_configurator.configure_slave_init_cmds(idx, eni::EsmTransition::SO);
    (void)m_configurator.configure_dc(idx);
    if (!m_configurator.request_slave_state(idx, states::OP)) {
      group.state = HotConnectGroupState::ERROR_STATE;
      return false;
    } else {
      // Reached OP
    }
  }

  group.state = HotConnectGroupState::OPERATIONAL;
  group.missed_cycles = 0;
  return true;
}

void HotConnectManager::scan_cycle() {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return;
  } else {
    // Lock acquired
  }

  ++m_cycleCounter;

  for (HotConnectGroupConfig &grp : m_groups) {
    if (grp.state == HotConnectGroupState::DISCONNECTED ||
        grp.state == HotConnectGroupState::ERROR_STATE) {
      if (probe_and_identify_group(grp)) {
        grp.state = HotConnectGroupState::DETECTED;
        (void)initialize_group_non_intrusive(grp);
      } else {
        // Still absent
      }
    } else if (grp.state == HotConnectGroupState::OPERATIONAL) {
      if (!check_group_health(grp)) {
        ++grp.missed_cycles;
        if (grp.missed_cycles >= 2) {
          grp.state = HotConnectGroupState::DISCONNECTED;
          grp.missed_cycles = 0;
        } else {
          // Warning state
        }
      } else {
        grp.missed_cycles = 0;
      }
    } else {
      // In transition
    }
  }
}

void HotConnectManager::mark_group_disconnected(const std::string &name) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    for (HotConnectGroupConfig &grp : m_groups) {
      if (grp.name == name) {
        grp.state = HotConnectGroupState::DISCONNECTED;
        grp.missed_cycles = 0;
        break;
      } else {
        // continue search
      }
    }
  } else {
    // Lock failed
  }
}

std::size_t HotConnectManager::operational_group_count() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return 0;
  } else {
    std::size_t count = 0;
    for (const HotConnectGroupConfig &grp : m_groups) {
      if (grp.state == HotConnectGroupState::OPERATIONAL) {
        ++count;
      } else {
        // Not operational
      }
    }
    return count;
  }
}

std::size_t HotConnectManager::total_group_count() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return 0;
  } else {
    return m_groups.size();
  }
}

} // namespace resoem::hot_connect
