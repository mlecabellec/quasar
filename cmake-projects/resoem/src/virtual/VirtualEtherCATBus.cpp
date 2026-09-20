/**
 * @file VirtualEtherCATBus.cpp
 * @brief Implementation of in-memory simulated EtherCAT fieldbus.
 * @details Contributes to [TSK-20260920-001.11].
 */

#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include "resoem/EtherCATFrame.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>

namespace resoem::virtual_bus {

namespace {

constexpr std::size_t MAX_DATAGRAM_ITERATIONS = 128;
constexpr std::size_t MAX_SLAVE_ITERATIONS = 512;

} // anonymous namespace

std::shared_ptr<VirtualEtherCATBus> VirtualEtherCATBus::create() {
  struct MakeSharedEnabler : public VirtualEtherCATBus {
    MakeSharedEnabler() : VirtualEtherCATBus() {}
  };
  return std::make_shared<MakeSharedEnabler>();
}

void VirtualEtherCATBus::attach_slave(std::shared_ptr<SimulatedSlave> slave) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    if (slave != nullptr) {
      m_slaves.push_back(slave);
    } else {
      // Ignore null
    }
  } else {
    // Lock failed
  }
}

void VirtualEtherCATBus::clear_slaves() {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    m_slaves.clear();
    m_brokenAfterSlave.reset();
  } else {
    // Lock failed
  }
}

std::size_t VirtualEtherCATBus::slave_count() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_slaves.size();
  } else {
    return 0;
  }
}

std::shared_ptr<SimulatedSlave> VirtualEtherCATBus::get_slave(std::size_t index) const {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    if (index < m_slaves.size()) {
      return m_slaves[index];
    } else {
      return nullptr;
    }
  } else {
    return nullptr;
  }
}

void VirtualEtherCATBus::break_cable_after(std::size_t slave_index) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    m_brokenAfterSlave = slave_index;
  } else {
    // Lock failed
  }
}

void VirtualEtherCATBus::restore_cable() {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    m_brokenAfterSlave.reset();
  } else {
    // Lock failed
  }
}

bool VirtualEtherCATBus::is_cable_broken() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_brokenAfterSlave.has_value();
  } else {
    return false;
  }
}

void VirtualEtherCATBus::process_frame(std::span<const byte> tx_frame, bool from_secondary,
                                       std::vector<byte> &rx_frame) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(500));
  if (!lock.owns_lock()) {
    rx_frame.assign(tx_frame.begin(), tx_frame.end());
    return;
  } else {
    // Lock acquired
  }

  rx_frame.assign(tx_frame.begin(), tx_frame.end());

  if (rx_frame.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE) {
    return;
  } else {
    // Valid minimum size
  }

  // Verify EtherType == 0x88A4
  if (rx_frame[12] != 0x88 || rx_frame[13] != 0xA4) {
    return;
  } else {
    // Valid EtherCAT frame
  }

  // Parse EtherCAT frame header
  std::uint16_t ecat_hdr = 0;
  std::memcpy(&ecat_hdr, &rx_frame[ETHERNET_HEADER_SIZE], sizeof(std::uint16_t));
  std::uint16_t total_ecat_len = ecat_hdr & 0x7FF;

  // Determine list of reachable slave indices in order of traversal
  std::vector<std::size_t> reachable_indices;
  if (!m_slaves.empty()) {
    if (!from_secondary) {
      // Primary port transmission (forward from slave 0)
      std::size_t max_idx = m_slaves.size() - 1;
      if (m_brokenAfterSlave.has_value()) {
        max_idx = std::min(m_brokenAfterSlave.value(), max_idx);
      } else {
        // Normal bus
      }
      for (std::size_t i = 0; i <= max_idx && i < MAX_SLAVE_ITERATIONS; ++i) {
        reachable_indices.push_back(i);
      }
    } else {
      // Secondary port transmission (reverse from last slave)
      if (m_brokenAfterSlave.has_value()) {
        std::size_t min_idx = m_brokenAfterSlave.value() + 1;
        if (min_idx < m_slaves.size()) {
          for (std::size_t i = m_slaves.size(); i > min_idx && (m_slaves.size() - i) < MAX_SLAVE_ITERATIONS; --i) {
            reachable_indices.push_back(i - 1);
          }
        } else {
          // Break is after last slave
        }
      } else {
        // No break: secondary frame traverses in reverse
        for (std::size_t i = m_slaves.size(); i > 0 && (m_slaves.size() - i) < MAX_SLAVE_ITERATIONS; --i) {
          reachable_indices.push_back(i - 1);
        }
      }
    }
  } else {
    // No slaves attached
  }

  // Traverse datagrams
  std::size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  std::size_t end_offset = std::min(rx_frame.size(), offset + total_ecat_len);

  for (std::size_t iter = 0; iter < MAX_DATAGRAM_ITERATIONS; ++iter) {
    if (offset + 10 + 2 > end_offset) {
      break;
    } else {
      // Continue parsing datagram
    }

    std::uint8_t cmd = rx_frame[offset + 0];
    std::uint16_t addr = 0;
    std::memcpy(&addr, &rx_frame[offset + 2], sizeof(std::uint16_t));
    std::uint16_t reg_off = 0;
    std::memcpy(&reg_off, &rx_frame[offset + 4], sizeof(std::uint16_t));
    std::uint16_t len_field = 0;
    std::memcpy(&len_field, &rx_frame[offset + 6], sizeof(std::uint16_t));

    std::size_t dgram_len = len_field & 0x7FF;
    bool more_bit = (len_field & 0x8000) != 0;

    if (offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE > end_offset) {
      break;
    } else {
      // Full datagram fits
    }

    std::span<byte> payload(&rx_frame[offset + DATAGRAM_HEADER_SIZE], dgram_len);
    std::uint16_t wkc = 0;
    std::memcpy(&wkc, &rx_frame[offset + DATAGRAM_HEADER_SIZE + dgram_len], sizeof(std::uint16_t));

    std::uint32_t logical_addr = static_cast<std::uint32_t>(addr) |
                                 (static_cast<std::uint32_t>(reg_off) << 16);

    // Pass datagram through all reachable slaves
    for (std::size_t slave_idx : reachable_indices) {
      if (slave_idx < m_slaves.size() && m_slaves[slave_idx] != nullptr) {
        std::shared_ptr<SimulatedSlave> slave = m_slaves[slave_idx];
        slave->sync_inputs_from_named();

        // Dispatch datagram to slave according to EtherCAT command code
        if (cmd == cmds::APRD) {
          slave->process_aprd(addr, reg_off, payload, wkc);
        } else if (cmd == cmds::APWR) {
          slave->process_apwr(addr, reg_off, payload, wkc);
        } else if (cmd == cmds::FPRD) {
          // Configured address read
          slave->process_fprd(addr, reg_off, payload, wkc);
        } else if (cmd == cmds::FPWR) {
          // Configured address write
          slave->process_fpwr(addr, reg_off, payload, wkc);
        } else if (cmd == cmds::BRD) {
          // Broadcast read
          slave->process_brd(reg_off, payload, wkc);
        } else if (cmd == cmds::BWR) {
          // Broadcast write
          slave->process_bwr(reg_off, payload, wkc);
        } else if (cmd == cmds::LRD) {
          // Logical read
          slave->process_lrd(logical_addr, payload, wkc);
        } else if (cmd == cmds::LWR) {
          // Logical write
          slave->process_lwr(logical_addr, payload, wkc);
        } else if (cmd == cmds::LRW) {
          // Logical read/write
          slave->process_lrw(logical_addr, payload, wkc);
        } else {
          // Unhandled datagram command
        }
      } else {
        // Null or invalid index
      }
    }

    // Write back modified address (for auto-increment) and updated WKC
    std::memcpy(&rx_frame[offset + 2], &addr, sizeof(std::uint16_t));
    std::memcpy(&rx_frame[offset + DATAGRAM_HEADER_SIZE + dgram_len], &wkc, sizeof(std::uint16_t));

    offset += DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE;

    if (!more_bit) {
      break;
    } else {
      // More datagrams follow
    }
  }
}

} // namespace resoem::virtual_bus
