#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <vector>

namespace resoem {

class CoEHandler {
public:
  CoEHandler(MailboxHandler &mailbox);

  /**
   * SDO Download (Write to slave)
   */
  Result<>
  sdo_write(SlaveInfo &slave, uint16_t index, uint8_t subindex,
            std::span<const byte> data, bool complete_access = false,
            std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * SDO Upload (Read from slave)
   */
  Result<size_t>
  sdo_read(SlaveInfo &slave, uint16_t index, uint8_t subindex,
           std::span<byte> data,
           bool complete_access = false,
           std::chrono::microseconds timeout = std::chrono::seconds(2));

  struct ODEntry {
    uint16_t index;
    uint16_t datatype;
    uint8_t object_code;
    uint8_t max_subindex;
    std::string name;
  };

  /**
   * Read the list of all object indexes in the dictionary.
   */
  Result<std::vector<uint16_t>> read_od_list(
      SlaveInfo &slave,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * Read details for a specific object index.
   */
  Result<ODEntry> read_od_description(
      SlaveInfo &slave, uint16_t index,
      std::chrono::microseconds timeout = std::chrono::seconds(2));

private:
  MailboxHandler &mailbox_;

  void handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                            uint8_t subindex, uint32_t abort_code);
};

} // namespace resoem
