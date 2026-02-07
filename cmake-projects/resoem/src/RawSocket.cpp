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
  // Open raw socket for EtherCAT protocol
  sock_fd_ = socket(AF_PACKET, SOCK_RAW, htons(ETHERCAT_ETHERTYPE));
  if (sock_fd_ < 0) {
    throw SocketError("Failed to create raw socket: " +
                      std::string(strerror(errno)));
  }

  // Get interface index
  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);

  if (ioctl(sock_fd_, SIOCGIFINDEX, &ifr) < 0) {
    close(sock_fd_);
    throw SocketError("Failed to get interface index for " + interface_name +
                      ": " + strerror(errno));
  }
  if_index_ = ifr.ifr_ifindex;

  // Bind socket to interface
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

  // Default timeout: 100ms
  set_timeout(100);
}

RawSocket::~RawSocket() {
  if (sock_fd_ >= 0) {
    close(sock_fd_);
  }
}

RawSocket::RawSocket(RawSocket &&other) noexcept
    : interface_name_(std::move(other.interface_name_)),
      sock_fd_(other.sock_fd_), if_index_(other.if_index_) {
  other.sock_fd_ = -1;
}

RawSocket &RawSocket::operator=(RawSocket &&other) noexcept {
  if (this != &other) {
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

  ssize_t sent = ::send(sock_fd_, data.data(), data.size(), 0);
  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;
    throw SocketError("Failed to send data: " + std::string(strerror(errno)));
  }
  return static_cast<size_t>(sent);
}

size_t RawSocket::receive(std::span<byte> buffer) {
  if (sock_fd_ < 0)
    throw SocketError("Socket not open");

  ssize_t received = ::recv(sock_fd_, buffer.data(), buffer.size_bytes(), 0);
  if (received < 0) {
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

  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  if (setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    throw SocketError("Failed to set receive timeout: " +
                      std::string(strerror(errno)));
  }

  // Also set send timeout to avoid blocking forever
  if (setsockopt(sock_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
    throw SocketError("Failed to set send timeout: " +
                      std::string(strerror(errno)));
  }
}

std::array<uint8_t, 6> RawSocket::get_mac_address() const {
  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ - 1);

  if (ioctl(sock_fd_, SIOCGIFHWADDR, &ifr) < 0) {
    throw SocketError("Failed to get MAC address: " +
                      std::string(strerror(errno)));
  }

  std::array<uint8_t, 6> mac;
  std::memcpy(mac.data(), ifr.ifr_hwaddr.sa_data, 6);
  return mac;
}

} // namespace resoem
