#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <vector>

namespace resoem {

class EoEHandler {
public:
  EoEHandler(MailboxHandler &mailbox);

  /**
   * Send an Ethernet frame via EoE.
   */
  Result<> send_frame(SlaveInfo &slave, std::span<const byte> frame,
                     std::chrono::microseconds timeout = std::chrono::milliseconds(100));

  /**
   * Receive an Ethernet frame via EoE.
   */
  Result<std::vector<byte>> receive_frame(SlaveInfo &slave,
                        std::chrono::microseconds timeout = std::chrono::milliseconds(100));

private:
  MailboxHandler &mailbox_;
};

} // namespace resoem
