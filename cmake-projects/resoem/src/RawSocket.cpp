#include "resoem/RawSocket.hpp"
#include <cerrno>
#include <cstring>
#include <functional>
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

RawSocket::RawSocket(const std::string &interface_name,
                     const std::string &secondary_interface_name)
    : interface_name_(interface_name),
      secondary_interface_name_(secondary_interface_name) {
  open_socket(interface_name_, sock_fd_, if_index_);

  if (!secondary_interface_name_.empty()) {
    try {
      open_socket(secondary_interface_name_, sock_fd_secondary_,
                  if_index_secondary_);
    } catch (const SocketError &e) {
      // If secondary fails, we should probably close primary and throw? Or just
      // warn? For now throw.
      close(sock_fd_); // Cleanup primary
      throw;
    }
  }

  // Default timeout
  set_timeout(100);
}

void RawSocket::open_socket(const std::string &iface, int &fd, int &idx) {
  fd = socket(AF_PACKET, SOCK_RAW, htons(ETHERCAT_ETHERTYPE));
  if (fd < 0) {
    throw SocketError("Failed to create raw socket for " + iface + ": " +
                      std::string(strerror(errno)));
  }

  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    close(fd);
    throw SocketError("Failed to get interface index for " + iface + ": " +
                      strerror(errno));
  }
  idx = ifr.ifr_ifindex;

  struct sockaddr_ll sll;
  std::memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = idx;
  sll.sll_protocol = htons(ETHERCAT_ETHERTYPE);

  if (bind(fd, reinterpret_cast<struct sockaddr *>(&sll), sizeof(sll)) < 0) {
    close(fd);
    throw SocketError("Failed to bind socket to " + iface + ": " +
                      strerror(errno));
  }
}

RawSocket::~RawSocket() {
  if (sock_fd_ >= 0)
    close(sock_fd_);
  if (sock_fd_secondary_ >= 0)
    close(sock_fd_secondary_);
}

RawSocket::RawSocket(RawSocket &&other) noexcept
    : interface_name_(std::move(other.interface_name_)),
      secondary_interface_name_(std::move(other.secondary_interface_name_)),
      sock_fd_(other.sock_fd_), sock_fd_secondary_(other.sock_fd_secondary_),
      if_index_(other.if_index_),
      if_index_secondary_(other.if_index_secondary_) {
  other.sock_fd_ = -1;
  other.sock_fd_secondary_ = -1;
}

RawSocket &RawSocket::operator=(RawSocket &&other) noexcept {
  if (this != &other) {
    if (sock_fd_ >= 0)
      close(sock_fd_);
    if (sock_fd_secondary_ >= 0)
      close(sock_fd_secondary_);

    interface_name_ = std::move(other.interface_name_);
    secondary_interface_name_ = std::move(other.secondary_interface_name_);
    sock_fd_ = other.sock_fd_;
    sock_fd_secondary_ = other.sock_fd_secondary_;
    if_index_ = other.if_index_;
    if_index_secondary_ = other.if_index_secondary_;

    other.sock_fd_ = -1;
    other.sock_fd_secondary_ = -1;
  }
  return *this;
}

size_t RawSocket::send(std::span<const byte> data, bool use_secondary) {
  int fd = use_secondary ? sock_fd_secondary_ : sock_fd_;
  if (fd < 0)
    throw SocketError("Socket not open");

  ssize_t sent = ::send(fd, data.data(), data.size(), 0);
  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;
    throw SocketError("Failed to send data: " + std::string(strerror(errno)));
  }
  return static_cast<size_t>(sent);
}

size_t RawSocket::receive(std::span<byte> buffer, int *out_port_index) {
  if (sock_fd_ < 0)
    throw SocketError("Socket not open");

  // If redundancy is active, we need to poll both sockets.
  if (sock_fd_secondary_ >= 0) {
    struct timeval tv;
    socklen_t len = sizeof(tv);
    if (getsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, &len) < 0) {
      // Handle error or use default?
      tv.tv_sec = 0;
      tv.tv_usec = 100000; // 100ms default
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_fd_, &readfds);
    FD_SET(sock_fd_secondary_, &readfds);

    int max_fd = std::max(sock_fd_, sock_fd_secondary_);
    int ret = select(max_fd + 1, &readfds, nullptr, nullptr, &tv);

    if (ret > 0) {
      // Prioritize primary? Or check both?
      // Note: select modifies tv to reflect remaining time on Linux, but we
      // don't loop here yet.

      if (FD_ISSET(sock_fd_, &readfds)) {
        ssize_t r = ::recv(sock_fd_, buffer.data(), buffer.size_bytes(), 0);
        if (r > 0) {
          if (out_port_index)
            *out_port_index = 0;
          return static_cast<size_t>(r);
        }
      }
      if (FD_ISSET(sock_fd_secondary_, &readfds)) {
        ssize_t r =
            ::recv(sock_fd_secondary_, buffer.data(), buffer.size_bytes(), 0);
        if (r > 0) {
          if (out_port_index)
            *out_port_index = 1;
          return static_cast<size_t>(r);
        }
      }
      return 0; // Select said yes but recv returned <= 0?
    } else if (ret == 0) {
      return 0; // Timeout
    } else {
      if (errno == EINTR)
        return 0;
      throw SocketError("Select failed: " + std::string(strerror(errno)));
    }
  } else {
    // Single socket mode
    ssize_t received = ::recv(sock_fd_, buffer.data(), buffer.size_bytes(), 0);
    if (received < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        return 0;
      throw SocketError("Failed to receive data: " +
                        std::string(strerror(errno)));
    }
    if (out_port_index)
      *out_port_index = 0;
    return static_cast<size_t>(received);
  }
}

void RawSocket::set_timeout(int timeout_ms) {
  if (sock_fd_ < 0)
    return;

  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  std::function<void(int)> set_sock_timeout = [&](int fd) {
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      throw SocketError("Failed to set receive timeout: " +
                        std::string(strerror(errno)));
    }
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
      throw SocketError("Failed to set send timeout: " +
                        std::string(strerror(errno)));
    }
  };

  set_sock_timeout(sock_fd_);
  if (sock_fd_secondary_ >= 0) {
    set_sock_timeout(sock_fd_secondary_);
  }
}

std::array<uint8_t, 6> RawSocket::get_mac_address(bool use_secondary) const {
  int fd = use_secondary ? sock_fd_secondary_ : sock_fd_;
  if (fd < 0)
    throw SocketError("Socket not open");

  std::string iface =
      use_secondary ? secondary_interface_name_ : interface_name_;

  struct ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    throw SocketError("Failed to get MAC address: " +
                      std::string(strerror(errno)));
  }

  std::array<uint8_t, 6> mac;
  std::memcpy(mac.data(), ifr.ifr_hwaddr.sa_data, 6);
  return mac;
}

} // namespace resoem
