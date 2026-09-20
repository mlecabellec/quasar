/**
 * @file StubRawSocket.cpp
 * @brief Implementation of virtual in-memory IRawSocket for testing.
 * @details Contributes to [TSK-20260920-001.11].
 */

#include "resoem/virtual/StubRawSocket.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace resoem::virtual_bus {

StubRawSocket::StubRawSocket(std::shared_ptr<VirtualEtherCATBus> bus, bool enable_redundancy)
    : m_bus(std::move(bus)), m_hasRedundancy(enable_redundancy) {}

std::size_t StubRawSocket::send(std::span<const byte> data, bool use_secondary) {
  if (m_bus == nullptr) {
    throw SocketError("StubRawSocket: backing virtual bus is null");
  } else {
    // Valid bus
  }

  std::vector<byte> rx_frame;
  m_bus->process_frame(data, use_secondary, rx_frame);

  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    throw SocketError("StubRawSocket: lock timeout on send");
  } else {
    // Lock acquired
  }

  if (m_rxQueue.size() >= MAX_QUEUE_CAPACITY) {
    m_rxQueue.pop_front();
  } else {
    // Queue within capacity
  }

  int port_index = 0;
  if (m_hasRedundancy != false) {
    if (use_secondary != false) {
      if (m_bus->is_cable_broken() != false) {
        port_index = 1;
      } else {
        port_index = 0;
      }
    } else {
      port_index = 0;
    }
  } else {
    if (use_secondary != false) {
      port_index = 1;
    } else {
      port_index = 0;
    }
  }
  m_rxQueue.push_back({std::move(rx_frame), port_index});

  return data.size();
}

std::size_t StubRawSocket::receive(std::span<byte> buffer, int *out_port_index) {
  std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(200));
  if (!lock.owns_lock()) {
    return 0;
  } else {
    // Lock acquired
  }

  if (m_rxQueue.empty()) {
    return 0;
  } else {
    // Frame available
  }

  std::pair<std::vector<byte>, int> front_entry = std::move(m_rxQueue.front());
  m_rxQueue.pop_front();

  if (out_port_index != nullptr) {
    *out_port_index = front_entry.second;
  } else {
    // Caller does not care about port
  }

  std::size_t copy_len = std::min(buffer.size(), front_entry.first.size());
  std::memcpy(buffer.data(), front_entry.first.data(), copy_len);

  return copy_len;
}

void StubRawSocket::set_timeout(int timeout_ms) {
  m_timeoutMs = timeout_ms;
}

const std::string &StubRawSocket::interface_name() const noexcept {
  return m_ifName;
}

std::array<std::uint8_t, ETHERNET_MAC_ADDRESS_LENGTH> StubRawSocket::get_mac_address(bool use_secondary) const {
  if (use_secondary != false) {
    return {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
  } else {
    return {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
  }
}

bool StubRawSocket::has_redundancy() const noexcept {
  return m_hasRedundancy;
}

void StubRawSocket::set_redundancy(bool enable) noexcept {
  m_hasRedundancy = enable;
}

std::shared_ptr<VirtualEtherCATBus> StubRawSocket::get_bus() const noexcept {
  return m_bus;
}

} // namespace resoem::virtual_bus
