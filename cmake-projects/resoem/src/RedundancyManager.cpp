/**
 * @file RedundancyManager.cpp
 * @brief Implementation of EtherCAT Cable Redundancy loop topology engine.
 * @details Contributes to [TSK-20260920-001.5], [TSK-20260920-001.6], and [TSK-20260920-001.7].
 */

#include "resoem/RedundancyManager.hpp"
#include "resoem/EtherCATFrame.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <algorithm>
#include <cstring>

namespace resoem::redundancy {

namespace {

constexpr std::size_t MAX_DGRAM_LOOP = 64;
constexpr std::size_t DGRAM_LEN_OFFSET = 6;
constexpr std::size_t DGRAM_LEN_MASK = 0x07FF;
constexpr std::uint16_t DGRAM_MORE_FLAG = 0x8000;

} // anonymous namespace

RedundancyManager::RedundancyManager(IRawSocket &socket)
    : m_socket(socket) {}

RedundancyState RedundancyManager::state() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_state;
  } else {
    return RedundancyState::CIRCULAR_NORMAL;
  }
}

void RedundancyManager::set_expected_wkc(std::uint16_t expected_wkc) noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    m_expectedWkc = expected_wkc;
  } else {
    // Lock failed
  }
}

std::uint16_t RedundancyManager::expected_wkc() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_expectedWkc;
  } else {
    return 0;
  }
}

std::size_t RedundancyManager::break_count() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_breakCount;
  } else {
    return 0;
  }
}

std::size_t RedundancyManager::heal_count() const noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    return m_healCount;
  } else {
    return 0;
  }
}

void RedundancyManager::set_probe_interval_cycles(std::size_t cycles) noexcept {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (lock.owns_lock()) {
    m_probeIntervalCycles = cycles;
  } else {
    // Lock failed
  }
}

std::uint16_t RedundancyManager::extract_wkc_from_frame(std::span<const byte> frame) noexcept {
  if (frame.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE) {
    return 0;
  } else {
    // Valid size
  }

  std::size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  std::uint16_t last_wkc = 0;

  for (std::size_t i = 0; i < MAX_DGRAM_LOOP; ++i) {
    if (offset + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE > frame.size()) {
      break;
    } else {
      // Valid header
    }

    std::uint16_t len_field = 0;
    std::memcpy(&len_field, &frame[offset + DGRAM_LEN_OFFSET], sizeof(std::uint16_t));
    std::size_t dgram_len = len_field & DGRAM_LEN_MASK;
    bool more = (len_field & DGRAM_MORE_FLAG) != 0;

    if (offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE > frame.size()) {
      break;
    } else {
      // Valid datagram
    }

    std::uint16_t wkc = 0;
    std::memcpy(&wkc, &frame[offset + DATAGRAM_HEADER_SIZE + dgram_len], sizeof(std::uint16_t));
    last_wkc += wkc;

    offset += DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE;

    if (!more) {
      break;
    } else {
      // Next datagram
    }
  }

  return last_wkc;
}

void RedundancyManager::update_wkc_in_frame(std::span<byte> frame, std::uint16_t wkc) noexcept {
  if (frame.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE) {
    return;
  } else {
    // Valid size
  }

  std::size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  if (offset + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE <= frame.size()) {
    std::uint16_t len_field = 0;
    std::memcpy(&len_field, &frame[offset + DGRAM_LEN_OFFSET], sizeof(std::uint16_t));
    std::size_t dgram_len = len_field & DGRAM_LEN_MASK;

    if (offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE <= frame.size()) {
      std::memcpy(&frame[offset + DATAGRAM_HEADER_SIZE + dgram_len], &wkc, sizeof(std::uint16_t));
    } else {
      // Out of bounds
    }
  } else {
    // Out of bounds
  }
}

void RedundancyManager::merge_datagram_payloads(std::span<byte> target_frame,
                                                std::span<const byte> secondary_frame,
                                                std::span<const byte> original_tx) noexcept {
  if (target_frame.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE ||
      secondary_frame.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE ||
      original_tx.size() < ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE) {
    return;
  } else {
    // Valid sizes
  }

  std::size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  for (std::size_t iter = 0; iter < MAX_DGRAM_LOOP; ++iter) {
    if (offset + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE > target_frame.size() ||
        offset + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE > secondary_frame.size() ||
        offset + DATAGRAM_HEADER_SIZE + WORKING_COUNTER_SIZE > original_tx.size()) {
      break;
    } else {
      // Valid header
    }

    std::uint16_t len_field = 0;
    std::memcpy(&len_field, &target_frame[offset + DGRAM_LEN_OFFSET], sizeof(std::uint16_t));
    std::size_t dgram_len = len_field & DGRAM_LEN_MASK;
    bool more = (len_field & DGRAM_MORE_FLAG) != 0;

    if (offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE > target_frame.size() ||
        offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE > secondary_frame.size() ||
        offset + DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE > original_tx.size()) {
      break;
    } else {
      // Merge payloads
    }

    for (std::size_t b = 0; b < dgram_len; ++b) {
      std::size_t idx = offset + DATAGRAM_HEADER_SIZE + b;
      if (secondary_frame[idx] != original_tx[idx]) {
        if (target_frame[idx] != original_tx[idx]) {
          target_frame[idx] = static_cast<byte>(target_frame[idx] | secondary_frame[idx]);
        } else {
          target_frame[idx] = secondary_frame[idx];
        }
      } else {
        // Untouched on secondary: retain target_frame (from primary)
      }
    }

    offset += DATAGRAM_HEADER_SIZE + dgram_len + WORKING_COUNTER_SIZE;

    if (!more) {
      break;
    } else {
      // Continue
    }
  }
}

bool RedundancyManager::check_healing_probe() {
  // Transmit a lightweight BRD datagram on secondary interface
  FrameBuilder builder;
  std::array<byte, 1> probe_payload{0x00};
  builder.add_datagram(cmds::BRD, 0xEE, 0x0000, regs::TYPE, probe_payload);
  const std::span<const byte> PROBE_TX = builder.build();

  try {
    m_socket.send(PROBE_TX, true);
  } catch (const SocketError &) {
    m_consecutiveHealSuccess = 0;
    return false;
  }

  std::vector<byte> probe_rx(MAX_FRAME_SIZE, static_cast<byte>(0));
  int rx_port = -1;
  std::size_t rec = 0;
  try {
    rec = m_socket.receive(probe_rx, &rx_port);
  } catch (const SocketError &) {
    m_consecutiveHealSuccess = 0;
    return false;
  }

  if (rec >= ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE + DATAGRAM_HEADER_SIZE + 1 + WORKING_COUNTER_SIZE && rx_port == 0) {
    // Probe sent on secondary completed loop and arrived at primary!
    ++m_consecutiveHealSuccess;
    if (m_consecutiveHealSuccess >= DEFAULT_HEAL_CONFIRM_COUNT) {
      m_state = RedundancyState::CIRCULAR_NORMAL;
      ++m_healCount;
      m_consecutiveHealSuccess = 0;
      return true;
    } else {
      return false;
    }
  } else {
    m_consecutiveHealSuccess = 0;
    return false;
  }
}

Result<std::uint16_t> RedundancyManager::exchange_cyclic_frame(std::span<byte> frame_data,
                                                              std::uint16_t expected_wkc) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return std::unexpected(ECError::Timeout);
  } else {
    // Lock acquired
  }

  m_expectedWkc = expected_wkc;
  ++m_cycleCounter;

  if (m_state == RedundancyState::CIRCULAR_NORMAL) {
    // Transmit primary frame
    try {
      m_socket.send(frame_data, false);
    } catch (const SocketError &) {
      return std::unexpected(ECError::ProtocolError);
    }

    std::vector<byte> rx_buffer(frame_data.size(), static_cast<byte>(0));
    int rx_port = -1;
    std::size_t rec = 0;
    try {
      rec = m_socket.receive(rx_buffer, &rx_port);
    } catch (const SocketError &) {
      return std::unexpected(ECError::ProtocolError);
    }

    if (rec == 0) {
      return std::unexpected(ECError::Timeout);
    } else {
      // Frame received
    }

    std::uint16_t prim_wkc = extract_wkc_from_frame(rx_buffer);

    if (prim_wkc >= expected_wkc) {
      // Healthy circular ring: copy data back
      std::memcpy(frame_data.data(), rx_buffer.data(), std::min(frame_data.size(), rec));
      return prim_wkc;
    } else {
      // Cable break detected! Failover to bifurcated transmission
      m_state = RedundancyState::BIFURCATED_BROKEN;
      ++m_breakCount;

      // Retransmit on secondary port
      std::vector<byte> sec_tx(frame_data.begin(), frame_data.end());
      try {
        m_socket.send(sec_tx, true);
      } catch (const SocketError &) {
        return prim_wkc;
      }

      std::vector<byte> sec_rx(frame_data.size(), static_cast<byte>(0));
      int sec_port = -1;
      std::size_t sec_rec = 0;
      try {
        sec_rec = m_socket.receive(sec_rx, &sec_port);
      } catch (const SocketError &) {
        return prim_wkc;
      }
      (void)sec_rec;

      std::vector<byte> original_tx(frame_data.begin(), frame_data.end());
      std::uint16_t sec_wkc = extract_wkc_from_frame(sec_rx);
      std::memcpy(frame_data.data(), rx_buffer.data(), std::min(frame_data.size(), rec));
      merge_datagram_payloads(frame_data, sec_rx, original_tx);

      std::uint16_t total_wkc = prim_wkc + sec_wkc;
      update_wkc_in_frame(frame_data, total_wkc);
      return total_wkc;
    }
  } else if (m_state == RedundancyState::BIFURCATED_BROKEN ||
             m_state == RedundancyState::HEALING_PROBING) {
    std::vector<byte> original_tx(frame_data.begin(), frame_data.end());

    // Send on primary
    try {
      m_socket.send(frame_data, false);
    } catch (const SocketError &) {
      return std::unexpected(ECError::ProtocolError);
    }

    std::vector<byte> prim_rx(frame_data.size(), static_cast<byte>(0));
    int prim_port = -1;
    (void)m_socket.receive(prim_rx, &prim_port);
    std::uint16_t prim_wkc = extract_wkc_from_frame(prim_rx);

    // Send on secondary
    std::vector<byte> sec_tx(original_tx.begin(), original_tx.end());
    try {
      m_socket.send(sec_tx, true);
    } catch (const SocketError &) {
      return prim_wkc;
    }

    std::vector<byte> sec_rx(frame_data.size(), static_cast<byte>(0));
    int sec_port = -1;
    (void)m_socket.receive(sec_rx, &sec_port);
    std::uint16_t sec_wkc = extract_wkc_from_frame(sec_rx);

    std::memcpy(frame_data.data(), prim_rx.data(), frame_data.size());
    merge_datagram_payloads(frame_data, sec_rx, original_tx);

    std::uint16_t total_wkc = prim_wkc + sec_wkc;
    update_wkc_in_frame(frame_data, total_wkc);

    // Periodic healing check
    if (m_cycleCounter % m_probeIntervalCycles == 0) {
      check_healing_probe();
    } else {
      // Wait for next probe cycle
    }

    return total_wkc;
  } else {
    // Redundancy disabled
    return std::unexpected(ECError::ProtocolError);
  }
}

} // namespace resoem::redundancy
