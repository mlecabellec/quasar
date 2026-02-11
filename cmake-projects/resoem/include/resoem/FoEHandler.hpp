/**
 * @file FoEHandler.hpp
 * @brief File over EtherCAT (FoE) protocol implementation.
 */

#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace resoem {

/**
 * @brief Handles file transfers (Upload/Download) to/from EtherCAT slaves.
 * 
 * FoE is often used for firmware updates or reading/writing configuration files.
 */
class FoEHandler {
public:
  /**
   * @brief Construct a new FoE Handler object.
   * @param mailbox The low-level mailbox handler to use.
   */
  FoEHandler(MailboxHandler &mailbox);

  /**
   * @brief Download a file to the slave.
   * 
   * @param slave The target slave.
   * @param filename Destination filename on the slave.
   * @param password Optional password for the file access.
   * @param data File content to download.
   * @param timeout Maximum time to wait for the entire transfer.
   * @return Result<> Success or error code.
   */
  Result<> write_file(SlaveInfo &slave, std::string_view filename,
                     uint32_t password, std::span<const byte> data,
                     std::chrono::microseconds timeout = std::chrono::seconds(5));

  /**
   * @brief Upload a file from the slave.
   * 
   * @param slave The target slave.
   * @param filename Source filename on the slave.
   * @param password Optional password.
   * @param timeout Maximum time to wait.
   * @return Result<std::vector<byte>> The file content or error.
   */
  Result<std::vector<byte>> read_file(SlaveInfo &slave, std::string_view filename,
                    uint32_t password,
                    std::chrono::microseconds timeout = std::chrono::seconds(5));

private:
  MailboxHandler &mailbox_; ///< Low-level mailbox handler
};

} // namespace resoem
