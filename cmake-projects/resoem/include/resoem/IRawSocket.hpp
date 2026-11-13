/**
 * @file IRawSocket.hpp
 * @brief Abstract transport interface for EtherCAT frame communication.
 * @details Contributes to [FE-0040.1] and [TSK-20260920-001.11].
 */

#pragma once

#include "resoem/common.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace resoem {

/**
 * @class IRawSocket
 * @brief Abstract interface representing a physical or virtual EtherCAT network socket.
 * @details Allows polymorphic dispatch between physical Linux AF_PACKET sockets and
 * in-memory software test stubs (StubRawSocket).
 */
class IRawSocket {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~IRawSocket() noexcept = default;

  /// @brief Send a raw Ethernet frame.
  virtual std::size_t send(std::span<const byte> data, bool use_secondary = false) = 0;

  /// @brief Receive a raw Ethernet frame into a buffer.
  virtual std::size_t receive(std::span<byte> buffer, int *out_port_index = nullptr) = 0;

  /// @brief Set socket receive/send timeouts in milliseconds.
  virtual void set_timeout(int timeout_ms) = 0;

  /**
   * @brief Get the primary interface name string.
   * @return String reference (e.g. "eth0" or "virtual_primary").
   */
  [[nodiscard]] virtual const std::string &interface_name() const noexcept = 0;

  /**
   * @brief Get the MAC address of the specified port interface.
   * @param use_secondary If true, get MAC for the secondary interface.
   * @return Array containing the MAC address.
   */
  [[nodiscard]] virtual std::array<std::uint8_t, ETHERNET_MAC_ADDRESS_LENGTH> get_mac_address(bool use_secondary = false) const = 0;

  /**
   * @brief Check whether cable redundancy is active (secondary interface configured).
   * @return True if secondary interface is open.
   */
  [[nodiscard]] virtual bool has_redundancy() const noexcept = 0;
};

} // namespace resoem
