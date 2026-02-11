/**
 * @file RawSocket.hpp
 * @brief Raw Ethernet socket wrapper for Linux.
 */

#pragma once

#include "resoem/common.hpp"
#include <optional>
#include <string>
#include <vector>

namespace resoem {

/**
 * @brief Handles raw Ethernet packet communication.
 * 
 * This class provides a simple wrapper around Linux raw sockets (AF_PACKET)
 * specifically for EtherCAT (EtherType 0x88A4).
 */
class RawSocket {
public:
  /**
   * @brief Construct a new Raw Socket object.
   * @param interface_name The name of the network interface (e.g., "eth0").
   * @throws SocketError if the socket cannot be opened or bound.
   */
  RawSocket(const std::string &interface_name);

  /**
   * @brief Destroy the Raw Socket object and close the file descriptor.
   */
  ~RawSocket();

  // Prevent copying
  RawSocket(const RawSocket &) = delete;
  RawSocket &operator=(const RawSocket &) = delete;

  /**
   * @brief Move constructor.
   */
  RawSocket(RawSocket &&other) noexcept;

  /**
   * @brief Move assignment operator.
   */
  RawSocket &operator=(RawSocket &&other) noexcept;

  /**
   * @brief Send a raw frame.
   * @param data The data to send.
   * @return Number of bytes sent.
   * @throws SocketError on failure.
   */
  size_t send(std::span<const byte> data);

  /**
   * @brief Receive a raw frame.
   * @param buffer The buffer to store received data.
   * @return Number of bytes received.
   * @throws SocketError on failure.
   */
  size_t receive(std::span<byte> buffer);

  /**
   * @brief Set receive and send timeouts.
   * @param timeout_ms Timeout in milliseconds.
   */
  void set_timeout(int timeout_ms);

  /**
   * @brief Get the interface name.
   * @return The interface name string.
   */
  const std::string &interface_name() const { return interface_name_; }

  /**
   * @brief Get the MAC address of the bound interface.
   * @return 6-byte array containing the MAC address.
   * @throws SocketError if the MAC address cannot be retrieved.
   */
  std::array<uint8_t, 6> get_mac_address() const;

private:
  std::string interface_name_; ///< Name of the network interface
  int sock_fd_ = -1;           ///< Socket file descriptor
  int if_index_ = -1;          ///< Network interface index
};

} // namespace resoem
