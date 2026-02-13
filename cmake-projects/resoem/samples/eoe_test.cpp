/**
 * @file eoe_test.cpp
 * @brief Port of eoe_test.c to resoem
 */

#include "resoem/Enumerator.hpp"
#include "resoem/EoEHandler.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/RawSocket.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/if_tun.h>
#include <net/if.h>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace resoem;

// Minimal TAP interface handling
int tun_alloc(char *dev) {
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    return fd;

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (*dev)
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Usage: eoe_test IFNAME SLAVE_IDX\n";
    return 1;
  }

  std::string iface = argv[1];
  int slave_idx = std::stoi(argv[2]) - 1;

  try {
    RawSocket socket(iface);
    Enumerator enumerator(socket);

    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "No slaves found.\n";
      return 1;
    }

    if (slave_idx < 0 || slave_idx >= (int)enumerator.slaves().size()) {
      std::cout << "Invalid slave index.\n";
      return 1;
    }

    std::cout << "Configuring EoE on slave " << slave_idx + 1 << "...\n";

    // Setup TAP
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun0");
    int tun_fd = tun_alloc(tun_name);
    if (tun_fd < 0) {
      perror("Allocating interface");
      return 1;
    }
    std::cout << "Allocated TAP interface " << tun_name << "\n";
    std::cout << "Please configure IP on " << tun_name << " (e.g. ifconfig "
              << tun_name << " 192.168.1.100 up)\n";

    enumerator.request_state_all(states::SAFE_OP);
    // Need to be in OP for full EoE? Or PreOp/SafeOp sufficient?
    // Usually EoE works in PreOp too if mailbox is active.

    MailboxHandler mbx(socket);
    EoEHandler eoe(mbx);
    SlaveInfo &slave = const_cast<SlaveInfo &>(enumerator.slaves()[slave_idx]);

    std::cout << "Starting EoE tunneling loop... (Ctrl+C to stop)\n";

    while (true) {
      struct pollfd fds[1];
      fds[0].fd = tun_fd;
      fds[0].events = POLLIN;

      // Check for incoming packets from TAP to send to EtherCAT
      int ret = poll(fds, 1, 0); // Non-blocking
      if (ret > 0 && (fds[0].revents & POLLIN)) {
        std::vector<byte> frame(2048);
        int nread = read(tun_fd, frame.data(), frame.size());
        if (nread > 0) {
          frame.resize(nread);
          eoe.send_frame(slave, frame);
        }
      }

      // Check for incoming packets from EtherCAT to send to TAP
      auto res = eoe.receive_frame(slave, std::chrono::milliseconds(0));
      if (res) {
        write(tun_fd, res->data(), res->size());
      }

      // Maintain EtherCAT state?
      // If we are in OP, we need to send Process Data.
      // If in PreOp/SafeOp, we just do Mailbox.
      // Let's stay in SafeOp/PreOp for simplicity unless OP is required.

      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
