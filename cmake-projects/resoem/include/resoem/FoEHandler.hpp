#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace resoem {

class FoEHandler {
public:
  FoEHandler(MailboxHandler &mailbox);

  /**
   * Write file to slave (Download)
   */
  Result<> write_file(SlaveInfo &slave, std::string_view filename,
                     uint32_t password, std::span<const byte> data,
                     std::chrono::microseconds timeout = std::chrono::seconds(5));

  /**
   * Read file from slave (Upload)
   */
  Result<std::vector<byte>> read_file(SlaveInfo &slave, std::string_view filename,
                    uint32_t password,
                    std::chrono::microseconds timeout = std::chrono::seconds(5));

private:
  MailboxHandler &mailbox_;
};

} // namespace resoem
