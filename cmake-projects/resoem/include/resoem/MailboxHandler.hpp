#pragma once

#include "resoem/EtherCATTypes.hpp"
#include "resoem/RawSocket.hpp"
#include "resoem/Slave.hpp"
#include <chrono>
#include <span>

namespace resoem {

class MailboxHandler {
public:
  MailboxHandler(RawSocket &socket);

  /**
   * Write data to slave mailbox.
   * @return WKC (Working Counter)
   */
  int write(SlaveInfo &slave, mailbox::Type type, std::span<const byte> data,
            std::chrono::microseconds timeout = std::chrono::milliseconds(100));

  /**
   * Read data from slave mailbox.
   * @return WKC (Working Counter)
   */
  int read(SlaveInfo &slave, mailbox::Type &type, std::span<byte> data,
           size_t &actual_len,
           std::chrono::microseconds timeout = std::chrono::milliseconds(100));

private:
  RawSocket &socket_;
  uint8_t current_idx_ = 0;

  int send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                   std::span<byte> data);

  bool is_mailbox_full(const SlaveInfo &slave);
  bool is_mailbox_empty(const SlaveInfo &slave);

  template <typename T>
  T read_register_fprd(uint16_t configured_addr, uint16_t reg, int &wkc);
};

} // namespace resoem
