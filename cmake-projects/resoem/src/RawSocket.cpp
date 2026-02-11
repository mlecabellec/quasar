#include "resoem/RawSocket.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

namespace resoem {

RawSocket::RawSocket(const std::string &interface_name)
    : interface_name_(interface_name) {
  // Open raw socket for EtherCAT protocol. 
  // We use AF_PACKET to send/receive at the link layer, and SOCK_RAW 
  // to specify the protocol type (ETHERCAT_ETHERTYPE).
  sock_fd_ = socket(AF_PACKET, SOCK_RAW, htons(ETHERCAT_ETHERTYPE));
  if (sock_fd_ < 0) {
    throw SocketError("Failed to create raw socket: " +
                      std::string(strerror(errno)));
  }

  // Get interface index for the given interface name (e.g., "eth0").
  // This index is required for binding the socket to a specific hardware port.
  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);

  if (ioctl(sock_fd_, SIOCGIFINDEX, &ifr) < 0) {
    close(sock_fd_);
    throw SocketError("Failed to get interface index for " + interface_name +
                      ": " + strerror(errno));
  }
  if_index_ = ifr.ifr_ifindex;

  // Bind socket to the specified interface.
  // This ensures that we only send and receive frames through this network card.
  struct sockaddr_ll sll;
  std::memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = if_index_;
  sll.sll_protocol = htons(ETHERCAT_ETHERTYPE);

  if (bind(sock_fd_, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
    close(sock_fd_);
    throw SocketError("Failed to bind socket to " + interface_name + ": " +
                      strerror(errno));
  }

  // Default timeout: 100ms. EtherCAT usually expects much faster responses,
  // but this is a safe default for discovery and initialization.
  set_timeout(100);
}

RawSocket::~RawSocket() {
  // Clean up the socket resource.
  if (sock_fd_ >= 0) {
    close(sock_fd_);
  }
}

RawSocket::RawSocket(RawSocket &&other) noexcept
    : interface_name_(std::move(other.interface_name_)),
      sock_fd_(other.sock_fd_), if_index_(other.if_index_) {
  // Transfer ownership of the file descriptor and reset the source.
  other.sock_fd_ = -1;
}

RawSocket &RawSocket::operator=(RawSocket &&other) noexcept {
  if (this != &other) {
    // Close existing socket before taking over the new one.
    if (sock_fd_ >= 0) {
      close(sock_fd_);
    }
    interface_name_ = std::move(other.interface_name_);
    sock_fd_ = other.sock_fd_;
    if_index_ = other.if_index_;
    other.sock_fd_ = -1;
  }
  return *this;
}

size_t RawSocket::send(std::span<const byte> data) {
  if (sock_fd_ < 0)
    throw SocketError("Socket not open");

  // Send the raw data buffer. Since it's a raw socket, the data must
  // already contain the Ethernet header if we were using ETH_P_ALL,
  // but with our specific socket setup, the kernel helps with the header.
  ssize_t sent = ::send(sock_fd_, data.data(), data.size(), 0);
  if (sent < 0) {
    // If the send buffer is full, we return 0 instead of throwing to allow retries.
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;
    throw SocketError("Failed to send data: " + std::string(strerror(errno)));
  }
  return static_cast<size_t>(sent);
}

size_t RawSocket::receive(std::span<byte> buffer) {
  if (sock_fd_ < 0)
    throw SocketError("Socket not open");

  // Blocking receive of a single Ethernet frame.
  ssize_t received = ::recv(sock_fd_, buffer.data(), buffer.size_bytes(), 0);
  if (received < 0) {
    // Handle timeout or non-blocking cases.
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;
    throw SocketError("Failed to receive data: " +
                      std::string(strerror(errno)));
  }
  return static_cast<size_t>(received);
}

void RawSocket::set_timeout(int timeout_ms) {
  if (sock_fd_ < 0)
    return;

  // Convert milliseconds to timeval structure required by setsockopt.
  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  // Set receive timeout.
  if (setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    throw SocketError("Failed to set receive timeout: " +
                      std::string(strerror(errno)));
  }

  // Also set send timeout to avoid blocking forever if the hardware queue stalls.
  if (setsockopt(sock_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
    throw SocketError("Failed to set send timeout: " +
                      std::string(strerror(errno)));
  }
}

std::array<uint8_t, 6> RawSocket::get_mac_address() const {
  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ - 1);

  // Use ioctl to query the hardware address (MAC) of the interface.
  if (ioctl(sock_fd_, SIOCGIFHWADDR, &ifr) < 0) {
    throw SocketError("Failed to get MAC address: " +
                      std::string(strerror(errno)));
  }

  // Copy the MAC address from the socket structure to our array.
  std::array<uint8_t, 6> mac;
  std::memcpy(mac.data(), ifr.ifr_hwaddr.sa_data, 6);
  return mac;
}

} // namespace resoem
