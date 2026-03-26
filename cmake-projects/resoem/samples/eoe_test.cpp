/**
 * @file eoe_test.cpp
 * @brief Ethernet over EtherCAT (EoE) tunneling test (Resoem).
 * @details This sample demonstrates how to tunnel standard Ethernet traffic 
 * through an EtherCAT slave's mailbox. It creates a Linux TAP interface 
 * and bridges it to the EtherCAT bus.
 * 
 * Workflow:
 * 1. Initialize Network and discover slaves.
 * 2. Allocate a Linux TAP interface (e.g., tun0/tap0).
 * 3. Transition slaves to SAFE-OP (Mailbox active).
 * 4. Run a loop:
 *    - Poll TAP for outgoing Ethernet frames -> Send via EoE.
 *    - Check EoE for incoming Ethernet frames -> Write to TAP.
 */

#include "resoem/Enumerator.hpp"
#include "resoem/EoEHandler.hpp"
#include "resoem/MailboxHandler.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Diagnostics.hpp"
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
#include <signal.h>

using namespace resoem;

// Global flag for graceful exit
volatile sig_atomic_t g_stop = 0;
void handle_sigint(int) { g_stop = 1; }

/**
 * @brief Allocates a Linux TAP interface.
 * @param dev Name of the interface (e.g., "tap0").
 * @return File descriptor or -1 on error.
 */
int tun_alloc(char *dev) {
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    return fd;

  memset(&ifr, 0, sizeof(ifr));
  // IFF_TAP provides Ethernet-level frames (Layer 2)
  // IFF_NO_PI avoids the 4-byte protocol info header
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (*dev)
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Usage: eoe_test IFNAME SLAVE_IDX [-v]\n";
    std::cout << "Example: eoe_test eth0 1 -v\n";
    return 1;
  }

  signal(SIGINT, handle_sigint);
  std::string iface = argv[1];
  int slave_pos = std::stoi(argv[2]);
  bool verbose = (argc > 3 && std::string(argv[3]) == "-v");

  try {
    std::cout << "======================================================" << std::endl;
    std::cout << "EtherCAT EoE Tunneling Test (Resoem)" << std::endl;
    std::cout << "======================================================" << std::endl;

    std::cout << "[STEP 1] Initializing interface " << iface << "..." << std::endl;
    RawSocket socket(iface);
    Enumerator enumerator(socket);
    enumerator.set_verbose(verbose);

    if (auto res = enumerator.enumerate(); !res || *res == 0) {
      std::cout << "[ERROR] No slaves found on the bus.\n";
      return 1;
    }

    if (slave_pos < 1 || slave_pos > (int)enumerator.slaves().size()) {
      std::cout << "[ERROR] Slave index " << slave_pos << " out of range.\n";
      return 1;
    }

    int slave_idx = slave_pos - 1;
    SlaveInfo &slave = const_cast<SlaveInfo &>(enumerator.slaves()[slave_idx]);
    std::cout << "[INFO] Target Slave: " << slave.name << " (Addr: 0x" << std::hex << slave.configured_address << std::dec << ")\n";

    if (!(slave.mbx_protocols & 0x02)) {
        std::cout << "[WARNING] Slave SII does not claim EoE support.\n";
    }

    // Allocate TAP
    std::cout << "[STEP 2] Allocating Linux TAP interface..." << std::endl;
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "resoem_tap0");
    int tun_fd = tun_alloc(tun_name);
    if (tun_fd < 0) {
      std::cerr << "[ERROR] Failed to allocate TAP interface. Try running with sudo.\n";
      return 1;
    }
    std::cout << "[SUCCESS] Created TAP: " << tun_name << "\n";
    std::cout << "\n[INSTRUCTIONS] To test the tunnel, run these commands in another terminal:\n";
    std::cout << "  1. sudo ifconfig " << tun_name << " 192.168.1.100 up\n";
    std::cout << "  2. ping 192.168.1.1  (Assuming slave has IP 192.168.1.1)\n\n";

    std::cout << "[STEP 3] Moving slaves to SAFE-OP (Mailbox Active)..." << std::endl;
    enumerator.request_state_all(states::SAFE_OP);

    MailboxHandler mbx(socket);
    mbx.set_verbose(verbose);
    EoEHandler eoe(mbx);

    std::cout << "[STEP 4] Entering Tunneling Loop... (Ctrl+C to stop)\n";
    std::cout << "------------------------------------------------------" << std::endl;

    uint64_t tx_pkts = 0, rx_pkts = 0;
    while (!g_stop) {
      struct pollfd fds[1];
      fds[0].fd = tun_fd;
      fds[0].events = POLLIN;

      // 1. Check for incoming packets from TAP (OS -> EtherCAT)
      int ret = poll(fds, 1, 0); // Non-blocking check
      if (ret > 0 && (fds[0].revents & POLLIN)) {
        std::vector<byte> eth_frame(2048);
        ssize_t nread = read(tun_fd, eth_frame.data(), eth_frame.size());
        if (nread > 0) {
          eth_frame.resize(static_cast<size_t>(nread));
          if (eoe.send_frame(slave, eth_frame)) {
              tx_pkts++;
          } else {
              std::cerr << "  [ERROR] Failed to tunnel frame to slave.\n";
          }
        }
      }

      // 2. Check for incoming packets from EtherCAT (Slave -> OS)
      // We use a short timeout to keep the loop responsive
      auto res = eoe.receive_frame(slave, std::chrono::microseconds(100));
      if (res) {
        if (write(tun_fd, res->data(), res->size()) > 0) {
            rx_pkts++;
        }
      }

      if ((tx_pkts + rx_pkts) % 10 == 0 && (tx_pkts + rx_pkts) > 0) {
          printf("\rTraffic: TX=%lu pkts, RX=%lu pkts", tx_pkts, rx_pkts);
          fflush(stdout);
      }

      // Small sleep to prevent 100% CPU usage while maintaining low latency
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "\n------------------------------------------------------" << std::endl;
    std::cout << "[SHUTDOWN] Closing TAP and returning slaves to INIT..." << std::endl;
    close(tun_fd);
    enumerator.request_state_all(states::INIT);
    std::cout << "[INFO] Test finished." << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "[FATAL] " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
