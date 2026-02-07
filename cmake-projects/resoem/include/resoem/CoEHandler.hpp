#pragma once

#include "resoem/MailboxHandler.hpp"
#include <chrono>
#include <cstdint>
#include <expected>
#include <span>
#include <system_error>
#include <vector>

namespace resoem {

enum class CoEError {
  Success = 0,
  Timeout,
  MailboxError,
  SDOAbort,
  InvalidResponse,
  DataTooLarge
};

class CoEHandler {
public:
  CoEHandler(MailboxHandler &mailbox);

  /**
   * SDO Download (Write to slave)
   * Supports Expedited and Normal/Segmented transfers.
   */
  CoEError
  sdo_write(SlaveInfo &slave, uint16_t index, uint8_t subindex,
            std::span<const byte> data, bool complete_access = false,
            std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * SDO Upload (Read from slave)
   * Supports Expedited and Normal/Segmented transfers.
   */
  CoEError
  sdo_read(SlaveInfo &slave, uint16_t index, uint8_t subindex,
           std::span<byte> data, size_t &actual_size,
           bool complete_access = false,
           std::chrono::microseconds timeout = std::chrono::seconds(2));

private:
  MailboxHandler &mailbox_;

  CoEError handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                            uint8_t subindex, uint32_t abort_code);
};

} // namespace resoem
