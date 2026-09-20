/**
 * @file StubRawSocket.hpp
 * @brief Hardware-free in-memory IRawSocket implementation for automated CI testing.
 * @details Contributes to [TSK-20260920-001.11] Hardware-Free Virtual Testing Layer.
 */

#pragma once

#include "resoem/IRawSocket.hpp"
#include "resoem/virtual/VirtualBusConstants.hpp"
#include "resoem/virtual/VirtualEtherCATBus.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace resoem::virtual_bus {

/**
 * @class StubRawSocket
 * @brief Virtual in-memory implementation of IRawSocket.
 * @details Directs all frame traffic to a VirtualEtherCATBus, completely bypassing
 * Linux AF_PACKET sockets and root privilege requirements.
 */
class StubRawSocket : public IRawSocket {
public:
  /**
   * @brief Construct a new StubRawSocket.
   * @param bus Shared pointer to the virtual EtherCAT bus backing this socket.
   * @param enable_redundancy Whether secondary redundant interface is simulated as open.
   */
  explicit StubRawSocket(std::shared_ptr<VirtualEtherCATBus> bus, bool enable_redundancy = false);

  /**
   * @brief Destructor.
   */
  ~StubRawSocket() override = default;

  // Prevent copying
  StubRawSocket(const StubRawSocket &) = delete;
  StubRawSocket &operator=(const StubRawSocket &) = delete;

  /**
   * @brief Send a raw Ethernet frame into the virtual bus.
   * @param data Frame payload to send.
   * @param use_secondary If true, inject frame into secondary redundant interface.
   * @return Number of bytes transmitted.
   */
  std::size_t send(std::span<const byte> data, bool use_secondary = false) override;

  /**
   * @brief Receive a processed frame from the virtual bus.
   * @param buffer Output buffer.
   * @param out_port_index Optional pointer to record receiving interface (0=Primary, 1=Secondary).
   * @return Number of bytes received.
   */
  std::size_t receive(std::span<byte> buffer, int *out_port_index = nullptr) override;

  /**
   * @brief Set socket timeout in milliseconds.
   * @param timeout_ms Timeout value.
   */
  void set_timeout(int timeout_ms) override;

  /**
   * @brief Get interface name.
   * @return "virtual_eth0"
   */
  [[nodiscard]] const std::string &interface_name() const noexcept override;

  /**
   * @brief Get simulated MAC address.
   * @param use_secondary True for secondary interface.
   * @return Array containing the MAC address.
   */
  [[nodiscard]] std::array<std::uint8_t, ETHERNET_MAC_ADDRESS_LENGTH> get_mac_address(bool use_secondary = false) const override;

  /**
   * @brief Check if redundancy is enabled.
   * @return True if secondary interface is active.
   */
  [[nodiscard]] bool has_redundancy() const noexcept override;

  /**
   * @brief Configure whether the secondary redundant port is active.
   * @param enable Redundancy state.
   */
  void set_redundancy(bool enable) noexcept;

  /**
   * @brief Get access to the backing virtual bus.
   * @return Shared pointer to the virtual bus.
   */
  [[nodiscard]] std::shared_ptr<VirtualEtherCATBus> get_bus() const noexcept;

private:
  mutable std::timed_mutex m_mutex;                     ///< Concurrency guard
  std::shared_ptr<VirtualEtherCATBus> m_bus;             ///< Backing virtual bus
  std::string m_ifName{"virtual_eth0"};                  ///< Virtual interface name
  bool m_hasRedundancy{false};                          ///< Redundancy enabled flag
  int m_timeoutMs{DEFAULT_VIRTUAL_SOCKET_TIMEOUT_MS};    ///< Timeout in ms

  std::deque<std::pair<std::vector<byte>, int>> m_rxQueue; ///< Queue of received frames and port indices
};

} // namespace resoem::virtual_bus
