/**
 * @file CoEHandler.hpp
 * @brief CANopen over EtherCAT (CoE) protocol implementation.
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
 * @brief Handles CoE SDO (Service Data Object) uploads and downloads.
 * 
 * This class implements both expedited and segmented SDO transfers,
 * as well as Object Dictionary (OD) discovery.
 */
class CoEHandler {
public:
  /**
   * @brief Construct a new CoE Handler object.
   * @param mailbox The low-level mailbox handler to use.
   */
  explicit CoEHandler(MailboxHandler &mailbox);

  /**
   * @brief Perform an SDO Download (write to a slave's object dictionary).
   * 
   * Handles both expedited (<= 4 bytes) and segmented transfers automatically.
   * 
   * @param slave The target slave.
   * @param index Object index.
   * @param subindex Object sub-index.
   * @param data Data payload to write.
   * @param complete_access Whether to use Complete Access (writing all sub-indexes).
   * @param timeout Maximum time to wait for the transfer.
   * @return Result<> Success or error code.
   */
  Result<>
  sdo_write(SlaveInfo &slave, uint16_t index, uint8_t subindex,
            std::span<const byte> data, bool complete_access = false,
            std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * @brief Perform an SDO Upload (read from a slave's object dictionary).
   * 
   * Handles both expedited and segmented transfers.
   * 
   * @param slave The target slave.
   * @param index Object index.
   * @param subindex Object sub-index.
   * @param data Buffer to store received data.
   * @param complete_access Whether to use Complete Access.
   * @param timeout Maximum time to wait.
   * @return Result<size_t> Actual number of bytes read or error.
   */
  Result<size_t>
  sdo_read(SlaveInfo &slave, uint16_t index, uint8_t subindex,
           std::span<byte> data,
           bool complete_access = false,
           std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * @brief Represents an entry in the CANopen Object Dictionary.
   */
  struct ODEntry {
    uint16_t index;       ///< Object index
    uint16_t datatype;    ///< Data type index
    uint8_t object_code;  ///< Object code (e.g. VAR, ARRAY, RECORD)
    uint8_t max_subindex; ///< Highest sub-index supported
    std::string name;     ///< Object name
  };

  /**
   * @brief Read the list of all object indexes available in the slave's OD.
   * 
   * @param slave The target slave.
   * @param timeout Maximum time to wait.
   * @return Result<std::vector<uint16_t>> List of indexes or error.
   */
  Result<std::vector<uint16_t>> read_od_list(
      SlaveInfo &slave,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * @brief Read the detailed description of a specific OD entry.
   * 
   * @param slave The target slave.
   * @param index The object index to query.
   * @param timeout Maximum time to wait.
   * @return Result<ODEntry> The entry description or error.
   */
  Result<ODEntry> read_od_description(
      SlaveInfo &slave, uint16_t index,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * @brief Enable or disable verbose logging for CoE operations.
   * @param level Verbosity level.
   */
  void set_verbose(int level) { verbose_level_ = level; }

private:
  MailboxHandler &mailbox_;   ///< Low-level mailbox handler
  int verbose_level_ = 0;     ///< Verbose logging flag

  // Internal helper to log and handle SDO aborts.
  void handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                            uint8_t subindex, uint32_t abort_code);
};

} // namespace resoem
