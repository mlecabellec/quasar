/**
 * @file MailboxHandler.hpp
 * @brief Base class for EtherCAT mailbox protocol communication.
 */

#pragma once

#include "resoem/EtherCATTypes.hpp"
#include "resoem/IRawSocket.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include <chrono>
#include <mutex>
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
  /// @brief Construct a new Mailbox Handler object.
  explicit MailboxHandler(IRawSocket &socket);

  /// @brief Write data to the slave's output mailbox.
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

  /// @brief Enable or disable verbose logging for mailbox operations.
  void set_verbose(int level) { verbose_level_ = level; }

private:
  IRawSocket &socket_;       ///< Raw socket reference (physical or virtual)
  uint8_t current_idx_ = 0;  ///< Cyclic datagram index
  int retries_ = 3;          ///< Max transmission retries
  int verbose_level_ = 0;    ///< Verbose logging level
  mutable std::timed_mutex mutex_{}; ///< Guard concurrent mailbox socket access [CS-0010.46]

  /// @brief Send single mailbox datagram and wait for WKC response.
  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);

  /// @brief Check SM1 mailbox full flag via FPRD.
  bool is_mailbox_full(const SlaveInfo &slave);
  /// @brief Check SM0 mailbox empty flag via FPRD.
  bool is_mailbox_empty(const SlaveInfo &slave);

  /// @brief Read typed ESC register via configured address FPRD.
  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
};

} // namespace resoem
