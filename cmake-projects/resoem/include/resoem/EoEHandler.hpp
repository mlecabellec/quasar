/**
 * @file EoEHandler.hpp
 * @brief Ethernet over EtherCAT (EoE) protocol implementation.
 */

#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <vector>

namespace resoem {

/**
 * @brief Handles Ethernet frame tunneling over EtherCAT.
 * 
 * EoE allows standard Ethernet traffic (like TCP/IP) to be sent to a slave
 * through the EtherCAT mailbox.
 */
class EoEHandler {
public:
  /**
   * @brief Construct a new EoE Handler object.
   * @param mailbox The low-level mailbox handler to use.
   */
  explicit EoEHandler(MailboxHandler &mailbox);

  /**
   * @brief Send a standard Ethernet frame to the slave.
   * 
   * Handles fragmentation if the frame is larger than the mailbox capacity.
   * 
   * @param slave The target slave.
   * @param frame The raw Ethernet frame data.
   * @param timeout Maximum time to wait.
   * @return Result<> Success or error code.
   */
  Result<> send_frame(SlaveInfo &slave, std::span<const byte> frame,
                     std::chrono::microseconds timeout = std::chrono::milliseconds(100));

  /**
   * @brief Receive an Ethernet frame from the slave.
   * 
   * Handles reassembly of fragmented frames.
   * 
   * @param slave The target slave.
   * @param timeout Maximum time to wait.
   * @return Result<std::vector<byte>> The reassembled Ethernet frame or error.
   */
  Result<std::vector<byte>> receive_frame(SlaveInfo &slave,
                        std::chrono::microseconds timeout = std::chrono::milliseconds(100));

private:
  MailboxHandler &mailbox_; ///< Low-level mailbox handler
};

} // namespace resoem
