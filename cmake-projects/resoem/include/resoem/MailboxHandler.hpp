/**
 * @file MailboxHandler.hpp
 * @brief Base class for EtherCAT mailbox protocol communication.
 */

#pragma once

#include "resoem/EtherCATTypes.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include <chrono>
#include <span>

namespace resoem {

/**
 * @brief Handles low-level mailbox read and write operations.
 * 
 * This class implements the handshake required to send and receive mailbox
 * datagrams, including checking the SyncManager status bits for empty/full state.
 */
class MailboxHandler {
public:
  /**
   * @brief Construct a new Mailbox Handler object.
   * @param socket The raw socket for communication.
   */
  MailboxHandler(RawSocket &socket);

  /**
   * @brief Write data to the slave's output mailbox.
   * 
   * @param slave The target slave.
   * @param type Mailbox protocol type (CoE, FoE, etc.).
   * @param data Data payload to send.
   * @param timeout Maximum time to wait for the mailbox to become empty.
   * @return int Working Counter (WKC) on success, or <= 0 on failure.
   */
  int write(SlaveInfo &slave, mailbox::Type type, std::span<const byte> data,
            std::chrono::microseconds timeout = std::chrono::milliseconds(100));

  /**
   * @brief Read data from the slave's input mailbox.
   * 
   * @param slave The target slave.
   * @param type [out] Received mailbox protocol type.
   * @param data Buffer to store received payload.
   * @param actual_len [out] Actual number of bytes received.
   * @param timeout Maximum time to wait for the mailbox to become full.
   * @return int Working Counter (WKC) on success, or <= 0 on failure.
   */
  int read(SlaveInfo &slave, mailbox::Type &type, std::span<byte> data,
           size_t &actual_len,
           std::chrono::microseconds timeout = std::chrono::milliseconds(100));

  /**
   * @brief Enable or disable verbose logging for mailbox operations.
   * @param verbose True to enable.
   */
  void set_verbose(bool verbose) { verbose_ = verbose; }

private:
  RawSocket &socket_;        ///< Raw socket reference
  uint8_t current_idx_ = 0;  ///< Cyclic datagram index
  int retries_ = 3;          ///< Max transmission retries
  bool verbose_ = false;     ///< Verbose logging flag

  // Internal helper methods
  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);

  bool is_mailbox_full(const SlaveInfo &slave);
  bool is_mailbox_empty(const SlaveInfo &slave);

  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
};

} // namespace resoem
