/**
 * @file CoEHandler.hpp
 * @brief CANopen over EtherCAT (CoE) protocol implementation.
 */

#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <mutex>
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
  /// @brief Maximum segments permitted per single SDO segmented transfer [CS-0010.37].
  static constexpr size_t MAX_SDO_SEGMENTS = 16384U;
  /// @brief Maximum OD list pages permitted per discovery sequence [CS-0010.37].
  static constexpr size_t MAX_OD_LIST_PAGES = 4096U;

  /// @brief Construct a new CoE Handler object.
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

  /// @brief Perform an SDO Upload (read from a slave's object dictionary).
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

  /// @brief Read the list of all object indexes available in the slave's OD.
  Result<std::vector<uint16_t>> read_od_list(
      SlaveInfo &slave,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

  /// @brief Read the detailed description of a specific OD entry.
  Result<ODEntry> read_od_description(
      SlaveInfo &slave, uint16_t index,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

  /// @brief Enable or disable verbose logging for CoE operations.
  void set_verbose(int level) {
    std::unique_lock<std::timed_mutex> lock(mutex_, std::chrono::milliseconds(100));
    verbose_level_ = level;
  }

private:
  MailboxHandler &mailbox_;          ///< Low-level mailbox handler
  int verbose_level_ = 0;            ///< Verbose logging flag
  mutable std::timed_mutex mutex_{}; ///< Guard concurrent CoE transfers [CS-0010.46]

  /// @brief Internal helper to log and handle SDO aborts.
  void handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                        uint8_t subindex, uint32_t abort_code);
};

} // namespace resoem
