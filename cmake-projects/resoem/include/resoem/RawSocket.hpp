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
 * @details Contributes to [FE-0040.1] Transport Layer.
 *
 * This class provides a simple wrapper around Linux raw sockets (AF_PACKET)
 * specifically for EtherCAT (EtherType 0x88A4).
 */
class RawSocket {
public:
  /**
   * @brief Construct a new Raw Socket object.
   * @details [FE-0040.1.1] Uses Linux AF_PACKET sockets with SOCK_RAW.
   * @param interface_name The name of the primary network interface (e.g.,
   * "eth0").
   * @param secondary_interface_name Optional name of the secondary interface
   * (e.g., "eth1") for redundancy.
   * @throws SocketError if the socket cannot be opened or bound.
   */
  explicit RawSocket(const std::string &interface_name,
            const std::string &secondary_interface_name = "");

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
   * @details [FE-0040.1.3] Implement non-blocking send operation.
   * @param data The data to send.
   * @param use_secondary If true, send through the secondary interface.
   * @return Number of bytes sent.
   * @throws SocketError on failure.
   */
  size_t send(std::span<const byte> data, bool use_secondary = false);

  /**
   * @brief Receive a raw frame.
   * @details [FE-0040.1.3] Implement non-blocking receive operation.
   * @param buffer The buffer to store received data.
   * @param out_port_index Optional pointer to store the receiving port index
   * (0=Primary, 1=Secondary).
   * @return Number of bytes received.
   * @throws SocketError on failure.
   */
  size_t receive(std::span<byte> buffer, int *out_port_index = nullptr);

  /**
   * @brief Set receive and send timeouts.
   * @details [FE-0040.1.2] Support configurable receive timeouts.
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
   * @details [FE-0040.1.2] Interface-specific MAC address retrieval.
   * @param use_secondary If true, get the MAC of the secondary interface.
   * @return 6-byte array containing the MAC address.
   * @throws SocketError if the MAC address cannot be retrieved.
   */
  std::array<uint8_t, 6> get_mac_address(bool use_secondary = false) const;

  /**
   * @brief Check if redundancy is available (secondary interface configured).
   * @return true if secondary interface is active.
   */
  bool has_redundancy() const { return sock_fd_secondary_ >= 0; }

private:
  std::string interface_name_; ///< Name of the primary network interface
  std::string
      secondary_interface_name_; ///< Name of the secondary network interface
  int sock_fd_ = -1;             ///< Primary socket file descriptor
  int sock_fd_secondary_ = -1;   ///< Secondary socket file descriptor
  int if_index_ = -1;            ///< Primary interface index
  int if_index_secondary_ = -1;  ///< Secondary interface index

  void open_socket(const std::string &iface, int &fd, int &idx);
};

} // namespace resoem
